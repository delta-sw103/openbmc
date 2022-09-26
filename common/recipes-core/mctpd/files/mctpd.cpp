#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CLI/CLI.hpp"
#include <sstream>

#include <libmctp.h>
#include <libmctp-smbus.h>
#include <libmctp-asti3c.h>
#include <openbmc/ipmi.h>
#include "mctpd.hpp"
#include "mctpd_plat.hpp"
#include <cassert>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define __unused __attribute__((unused))
#define SYSFS_SLAVE_QUEUE "/sys/bus/i2c/devices/%d-10%02x/slave-mqueue"

// MCTP Interface
enum {
  MCTP_SMBUS = 0x1  // MCTP SMBUS
};

bool verbose = false;
static const mctp_eid_t local_eid_default = 8;
static char sockname[] = "mctp-mux";

struct i3c_data {
  struct mctp_binding_asti3c *asti3c;
  int in_fd;
  int out_fd;
};

struct binding {
  const char *name;
  int (*init)(struct mctp *mctp, struct binding *binding, mctp_eid_t eid, int n_params, char *const *params);
  int (*get_fdin)(struct binding *binding);
  int (*get_fdout)(struct binding *binding);
  int (*process)(struct binding *binding);
  void (*free)(struct binding *binding);
  void *data;
};

struct client {
  bool active;
  int sock;
  int msg_type;
  int msg_tag;
};

struct ctx {
  struct mctp *mctp;
  struct binding *binding;
  int local_eid;
  void *buf;
  size_t buf_size;

  int sock;
  struct pollfd *pollfds;

  struct client *clients;
  int n_clients;

  uint8_t tag_flags[8];
};


void showMsg (const char* msg) {
  if (verbose)
    std::cout << msg << std::endl;
}

void showMsgHexVal (const char* msg, int val) {
  if (verbose)
    std::cout << msg << "0x" << std::hex << val <<  std::endl;
}


static int get_mctp_msg_tag(uint8_t* tags_flag, uint8_t* tag_num) {
  int i = 0;

  for(i=0; i<8; i++) {
     if ( !( *tags_flag & (1 << i)) ) {
       *tags_flag |= 1 << i;
       *tag_num = i;
       return 0;
     }
  }
  fprintf(stderr, "%s not get tags\n", __func__);
  return -1;
}

static int del_mctp_msg_tag(struct ctx* ctx, uint8_t idx ) {
  struct client *client = &ctx->clients[idx];

  if (client->msg_type != -1 || client->msg_tag != -1) {
    ctx->tag_flags[client->msg_type] &= ~(1 << client->msg_tag);
  }
  fprintf(stderr, "%s ctx->tags_flag[%d] = %x\n", __func__, client->msg_type, ctx->tag_flags[client->msg_type]);
  return 0;
}

bool get_mctp_tag_owner(void* buf) {
  uint8_t type = *((uint8_t*) buf + 1);

  if ( type == MSG_TYPE_PLDM) {
    if (!!(*((uint8_t*) buf + 2) & 0x80)) {
      return true;
    }
  } else if (type == MSG_TYPE_NCSI) {
    return ((*((uint8_t*)buf + 6) & 0x80) == 0);
  }
  return false;
}

static void set_smbus_extera_params(struct ctx *ctx, uint8_t eid, struct mctp_smbus_pkt_private *smbus_params) {
  auto iter = EIDToSMBusSlaveAddr.find(eid);
  uint8_t addr = 0x64;

  if(iter != EIDToSMBusSlaveAddr.end()) {
    addr = iter->second;
    showMsgHexVal("Find, the value is ", addr);
  } else {
    showMsg("Do not Find");
  }

  smbus_params->mux_hold_timeout = 0;
  smbus_params->mux_flags = 0;
  smbus_params->fd = ctx->binding->get_fdout(ctx->binding);
  smbus_params->slave_addr = addr;
}

static void tx_message(struct ctx *ctx, mctp_eid_t eid, uint8_t *msg, size_t len, bool tag_owner, uint8_t tag) {

  std::string name = ctx->binding->name;
  if (name == "smbus") {
    struct mctp_smbus_pkt_private *smbus_params, _smbus_params;
    smbus_params = &_smbus_params;
    set_smbus_extera_params(ctx, eid, smbus_params);

    mctp_message_tx(ctx->mctp, eid, (void *)msg, len, tag_owner, tag, (void*)smbus_params);
  }
  else if (name == "asti3c") {
    struct mctp_asti3c_pkt_private pkt_private = { .fd = ctx->binding->get_fdout(ctx->binding) };

    if (mctp_message_tx(ctx->mctp, eid, (void *)msg, len, tag_owner, tag, &pkt_private) < 0) {
      std::cerr << "Fail to transfer MCTP message.\n";
    }
  }
  else {
    std::cerr<<"Error : missing binding name\n";
  }
}

static void client_remove_inactive(struct ctx *ctx)
{
  int i;

  for (i = 0; i < ctx->n_clients; i++) {
    struct client *client = &ctx->clients[i];
    if (client->active)
      continue;

    del_mctp_msg_tag(ctx, i);
    close(client->sock);

    ctx->n_clients--;
    memmove(&ctx->clients[i], &ctx->clients[i + 1], (ctx->n_clients - i) * sizeof(*ctx->clients));
    ctx->clients = (struct client*)realloc(ctx->clients, ctx->n_clients * sizeof(*ctx->clients));
  }
}

static void rx_message(uint8_t eid, void *data, void *msg, size_t len, bool tag_owner, uint8_t tag, void *prv)
{
  struct ctx *ctx = (struct ctx*)data;
  struct iovec iov[2];
  struct msghdr msghdr;
  bool removed = false;
  uint8_t msg_type = *(uint8_t *)msg;
  int i, rc;

  if (len < 2)
    return;

  if (verbose) {
    fprintf(stderr, "MCTP message received: len %zd, tag %d dest eid=%d\n", len, tag, eid);

    for (i=0; i < (int)len; i++) {
     fprintf(stderr, "%s msg[%d] =%x\n", __func__, i , *((char*)msg + i));
    }
  }

  memset(&msghdr, 0, sizeof(msghdr));
  msghdr.msg_iov = iov;
  msghdr.msg_iovlen = 2;
  iov[0].iov_base = &eid;
  iov[0].iov_len = 1;
  iov[1].iov_base = msg;
  iov[1].iov_len = len;

  for (i = 0; i < ctx->n_clients; i++) {
    struct client *client = &ctx->clients[i];

    if (client->msg_type != msg_type)
      continue;

    if (verbose)
      fprintf(stderr, "forwarding to client %d\n", i);

    rc = sendmsg(client->sock, &msghdr, 0);

    if (rc != (ssize_t)(len + 1)) {
      client->active = false;
      removed = true;
    }
  }

  if (removed)
    client_remove_inactive(ctx);
}

static int binding_asti3c_init (struct mctp *mctp, struct binding *binding,
                              mctp_eid_t eid, int n_params,
                              char *const *params __attribute__((unused)))
{
  if (n_params != 1) {
    warnx("i3c binding requires device param");
    return -1;
  }

  struct i3c_data *data;
  data = (struct i3c_data*) malloc (sizeof(*data));

  data->asti3c = mctp_asti3c_init();
  assert(data->asti3c);

  std::string i3c_dev_str = params[0];
  std::string dev = "/dev/bus/i3c/" + i3c_dev_str;

  data->out_fd = open(dev.c_str(), O_RDWR);
  if (data->out_fd < 0) {
    std::cerr << "Fail to open device. fd = " << data->out_fd << "\n";
    mctp_asti3c_free(data->asti3c);
    free(data);
    return -1;
  }
  std::cout << "i3c device path: " << dev << "\n";

  std::string mqueue_dev = "/sys/bus/i3c/devices/" + i3c_dev_str + "/ibi-mqueue";

  data->in_fd = open(mqueue_dev.c_str(), O_RDONLY);
  if (data->in_fd < 0) {
    std::cerr << "Fail to open device. fd = " << data->in_fd << "\n";
    mctp_asti3c_free(data->asti3c);
    free(data);
    return -1;
  }
  std::cout << "i3c mqueue device path: " << mqueue_dev << "\n";

  mctp_register_bus_dynamic_eid(mctp, &((data->asti3c)->binding));
  
  mctp_binding_set_tx_enabled(&((data->asti3c)->binding), true);

  binding->data = data;
  return 0;
}

static int binding_asti3c_get_in_fd(struct binding *binding) {
  struct i3c_data *data = (struct i3c_data*)binding->data;

  return data->in_fd;
}

static int binding_asti3c_get_out_fd(struct binding *binding) {
  struct i3c_data *data = (struct i3c_data*)binding->data;
  return data->out_fd;
}

static int binding_asti3c_process(struct binding *binding) {
  struct i3c_data *data = (struct i3c_data*)binding->data;
  return mctp_asti3c_rx(data->asti3c, data->in_fd);
}

static void binding_asti3c_free(struct binding *binding) {
  struct i3c_data *data = (struct i3c_data*)binding->data;
  mctp_asti3c_free(data->asti3c);
  free(data);
}

static int binding_smbus_init(struct mctp *mctp, struct binding *binding,
                              mctp_eid_t eid, int n_params,
                              char *const *params __attribute__((unused)))
{
  struct mctp_binding_smbus *smbus;
  int bus;
  int addr;
  int rc;
  int fd;

  std::stringstream slave_queue;

  if (n_params != 2) {
    warnx("smbus binding requires device param");
    return -1;
  }

  bus = atoi(params[0]);
  addr = strtol(params[1], NULL, 16);
  smbus = mctp_smbus_init();
  assert(smbus);
  mctp_smbus_set_src_slave_addr(smbus, (addr << 1) + 1);

  std::string dev = "/dev/i2c-" + std::to_string(bus);
  printf("dev %s\n", dev.c_str());
  fd = open(dev.c_str(), O_RDWR);
  if (fd < 0)
    goto bail;

  mctp_smbus_set_out_fd(smbus, fd);

  //Get fd
  slave_queue << "/sys/bus/i2c/devices/"<< (int)bus
              << "-10" << std::hex << (int)addr
              << "/slave-mqueue";

  fd = open(slave_queue.str().c_str(), O_RDONLY);
  if (fd < 0) {
    printf("%s: open %s failed", __func__, slave_queue.str().c_str());
    goto bail;
  }

  mctp_smbus_set_in_fd(smbus, fd);
  rc = mctp_smbus_register_bus(smbus, mctp, eid);

  if (rc != 0)
    goto bail;

  binding->data = smbus;
  return 0;

bail:
  mctp_smbus_free(smbus);
  return -1;
}

static int binding_smbus_get_in_fd(struct binding *binding)
{
  struct mctp_binding_smbus* smbus = (struct mctp_binding_smbus*)binding->data;
  return smbus->in_fd;
}

static int binding_smbus_get_out_fd(struct binding *binding)
{
  struct mctp_binding_smbus* smbus = (struct mctp_binding_smbus*)binding->data;
  return smbus->out_fd;
}

static int binding_smbus_process(struct binding *binding)
{
  return mctp_smbus_read((struct mctp_binding_smbus*)binding->data);
}

static void binding_smbus_free(struct binding *binding) {
  struct mctp_binding_smbus* smbus = (struct mctp_binding_smbus*)binding->data;
  mctp_smbus_free(smbus);
}

struct binding bindings[] = {
  {
    .name = "smbus",
    .init = binding_smbus_init,
    .get_fdin = binding_smbus_get_in_fd,
    .get_fdout = binding_smbus_get_out_fd,
    .process = binding_smbus_process,
    .free = binding_smbus_free,
  },
  {
    .name = "asti3c",
    .init = binding_asti3c_init,
    .get_fdin = binding_asti3c_get_in_fd,
    .get_fdout = binding_asti3c_get_out_fd,
    .process = binding_asti3c_process,
    .free = binding_asti3c_free,
  }
};

struct binding *binding_lookup(const char *name)
{
  struct binding *binding;
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE(bindings); i++) {
    binding = &bindings[i];

    if (!strcmp(binding->name, name))
      return binding;
  }

  return NULL;
}

static int socket_init(struct ctx *ctx, char *const *argv)
{
  struct sockaddr_un addr;
  int namelen, rc;
  std::string path = sockname + std::string(argv[0]);

  namelen = path.length();
  memcpy(addr.sun_path, "\0", 1);
  memcpy(addr.sun_path+1, path.c_str(), namelen++);

  addr.sun_family = AF_UNIX;
  ctx->sock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (ctx->sock < 0) {
    warn("can't create socket");
    return -1;
  }

  rc = bind(ctx->sock, (struct sockaddr *)&addr, sizeof(addr.sun_family) + namelen);
  if (rc) {
    warn("can't bind socket");
    goto err_close;
  }

  rc = listen(ctx->sock, 1);
  if (rc) {
    warn("can't listen on socket");
    goto err_close;
  }

  return 0;

err_close:
  close(ctx->sock);
  return -1;
}

static int socket_process(struct ctx *ctx)
{
  struct client *client;
  int fd;

  fd = accept4(ctx->sock, NULL, 0, SOCK_NONBLOCK);
  if (fd < 0)
    return -1;

  ctx->n_clients++;
  ctx->clients = (struct client*)realloc(ctx->clients, ctx->n_clients * sizeof(struct client));

  client = &ctx->clients[ctx->n_clients - 1];
  memset(client, 0, sizeof(*client));
  client->active = true;
  client->sock = fd;
  client->msg_type = -1;
  client->msg_tag = -1;

  return 0;
}

static int client_process_recv(struct ctx *ctx, int idx)
{
  struct client *client = &ctx->clients[idx];
  uint8_t dest_eid;
  uint8_t tag_num=0;
  bool tag_owner=false;
  ssize_t len;
  int rc;

  if (client->msg_type < 0) {
    uint8_t type;
    rc = read(client->sock, &type, 1);
    if (rc <= 0)
      goto out_close;

    client->msg_type = type;
    rc = get_mctp_msg_tag(&ctx->tag_flags[client->msg_type], &tag_num);

    if (verbose) {
      fprintf(stderr, "client[%d] registered for type %u\n", idx, type);
      fprintf(stderr, "rc=%d tags_num=%d tags_flag = 0x%x\n", rc, tag_num, ctx->tag_flags[client->msg_type]);
    }

    if ( rc != 0 )
      return -1;

    client->msg_tag = tag_num;
    return 0;
  }

  len = recv(client->sock, NULL, 0, MSG_PEEK | MSG_TRUNC);
  if (len < 0) {
    if (errno != ECONNRESET)
      warn("can't receive (peek) from client");

    rc = -1;
    goto out_close;
        }

  if (len > (ssize_t)ctx->buf_size) {
    void *tmp;

    tmp = realloc(ctx->buf, len);
    if (!tmp) {
      warn("can't allocate for incoming message");
      rc = -1;
      goto out_close;
    }
    ctx->buf = tmp;
    ctx->buf_size = len;
  }

  rc = recv(client->sock, ctx->buf, ctx->buf_size, 0);
  if (rc < 0) {
    if (errno != ECONNRESET)
      warn("can't receive from client");
    rc = -1;
    goto out_close;
  }

  if (rc <= 0) {
    rc = -1;
    goto out_close;
  }

  dest_eid = *(uint8_t *)ctx->buf; 

  if (verbose) {
    fprintf(stderr, "client[%d] sent message: dest 0x%02x len %d\n", idx, dest_eid, rc - 1);
    for (int i=0; i<rc; i++) {
      printf("tx_msg[%d]=%x\n", i , *((uint8_t*)ctx->buf + i));
    }
  }

  if (dest_eid == ctx->local_eid) {
    //Loop back Test
    rx_message(dest_eid, ctx, (uint8_t *)ctx->buf + 1, rc - 1, 0, 0, NULL);
  } else {
    tag_owner = get_mctp_tag_owner(ctx->buf);
    tx_message(ctx, dest_eid, (uint8_t *)ctx->buf + 1, rc - 1,  tag_owner, client->msg_tag);
  }
  return 0;

out_close:
  client->active = false;
  return rc;
}

static int binding_init(struct ctx *ctx, const char *name, int argc, char *const *argv)
{
  int rc;

  ctx->binding = binding_lookup(name);
  if (!ctx->binding) {
    warnx("no such binding '%s'", name);
    return -1;
  }

  rc = ctx->binding->init(ctx->mctp, ctx->binding, ctx->local_eid, argc, argv);

  return rc;
}

enum {
  FD_BINDING = 0,
  FD_SOCKET,
  FD_NR,
};

static int run_daemon(struct ctx *ctx)
{
  bool clients_changed = false;
  int rc, i;
  int timeout_msecs = 200;

  ctx->pollfds = (struct pollfd*)malloc(FD_NR * sizeof(struct pollfd));

  if (ctx->binding->get_fdin) {
    ctx->pollfds[FD_BINDING].fd = ctx->binding->get_fdin(ctx->binding);
    ctx->pollfds[FD_BINDING].events = POLLPRI;
  } else {
    ctx->pollfds[FD_BINDING].fd = -1;
    ctx->pollfds[FD_BINDING].events = 0;
  }

  ctx->pollfds[FD_SOCKET].fd = ctx->sock;
  ctx->pollfds[FD_SOCKET].events = POLLIN;

  mctp_set_rx_all(ctx->mctp, rx_message, ctx);

  for (;;) {
    if (clients_changed) {
      int i;

      ctx->pollfds = (struct pollfd*)realloc(ctx->pollfds, (ctx->n_clients + FD_NR) * sizeof(struct pollfd));

      for (i = 0; i < ctx->n_clients; i++) {
        ctx->pollfds[FD_NR + i].fd = ctx->clients[i].sock;
        ctx->pollfds[FD_NR + i].events = POLLIN;
      }
      clients_changed = false;
    }

    rc = poll(ctx->pollfds, ctx->n_clients + FD_NR, timeout_msecs);
    if (rc < 0) {
      fprintf(stderr,"poll failed");
      break;
    }

    if (!rc)
      continue;

    if (ctx->pollfds[FD_BINDING].revents) {
      rc = 0;
      if (ctx->binding->process) {
        rc = ctx->binding->process(ctx->binding);
        if (rc) {
          continue;
        }
      }
    }

    for (i = 0; i < ctx->n_clients; i++) {
      if (!ctx->pollfds[FD_NR + i].revents)
        continue;
      rc = client_process_recv(ctx, i);
      if (rc)
        clients_changed = true;
    }

    if (ctx->pollfds[FD_SOCKET].revents) {
      rc = socket_process(ctx);
      if (rc)
        break;
      clients_changed = true;
    }
    if (clients_changed)
      client_remove_inactive(ctx);
  }

  free(ctx->pollfds);
  return rc;
}

static const struct option options[] = {
  { "verbose", no_argument, 0, 'v' },
  { "eid", required_argument, 0, 'e' },
  { "help", no_argument, 0, 'h' },
  { 0 },
};

static void usage(const char *progname)
{
  unsigned int i;

  fprintf(stderr, "usage: %s <binding> [params]\n", progname);
  fprintf(stderr, "Available bindings:\n");
  for (i = 0; i < ARRAY_SIZE(bindings); i++) {
    switch (i) {
    case MCTP_OVER_SMBUS: //smbus
      fprintf(stderr, " %s %s [bus] [bmc_addr_16h]\n", progname, bindings[i].name);
      break;
    case MCTP_OVER_I3C:
      fprintf(stderr, " %s %s [bus-pid]\n", progname, bindings[i].name);
    }
  }
}

int main(int argc, char *const *argv)
{
  struct ctx *ctx, _ctx;
  int rc;

  ctx = &_ctx;
  ctx->clients = NULL;
  ctx->n_clients = 0;
  ctx->local_eid = local_eid_default;
  memset(ctx->tag_flags, 0, sizeof(ctx->tag_flags));

  for (;;) {
    rc = getopt_long(argc, argv, "e:v", options, NULL);
    if (rc == -1)
      break;
    switch (rc) {
    case 'v':
      verbose = true;
      break;
    case 'e':
      ctx->local_eid = atoi(optarg);
      break;
    case 'h':
      usage(argv[0]);
    default:
      fprintf(stderr, "Invalid argument\n");
      return EXIT_FAILURE;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "missing binding argument\n");
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  //  setup initial buffer
  ctx->buf_size = 4096;
  ctx->buf = malloc(ctx->buf_size);
  memset(ctx->buf, 0, ctx->buf_size);

  if (verbose)
    mctp_set_log_stdio(MCTP_LOG_DEBUG);

  ctx->mctp = mctp_init();
  assert(ctx->mctp);

  rc = binding_init(ctx, argv[optind], argc - optind - 1,  argv + optind + 1);
  if (rc) {
    return EXIT_FAILURE;
  }

  rc = socket_init(ctx, argv + optind + 1);
  if (rc) {
    goto fail;
  }

  rc = run_daemon(ctx);
  if (rc) {
    goto fail;
  }

  return EXIT_SUCCESS;
fail:
  ctx->binding->free(ctx->binding);
  return EXIT_FAILURE;
}
