#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <facebook/bic_ipmi.h>
#include <facebook/bic_xfer.h>
#include "raa_gen3.h"
#include "xdpe152xx.h"
#include "tps53688.h"


static char fru_name[64] = {0};  // prefix of vr version cache (kv)

static int
fbwc_vr_rdwr(uint8_t bus, uint8_t addr, uint8_t *txbuf, uint8_t txlen,
              uint8_t *rxbuf, uint8_t rxlen) {
  uint8_t tbuf[64] = {0};

  if (txbuf == NULL || rxbuf == NULL) {
    return -1;
  }

  if ((txlen + 3) > sizeof(tbuf)) {
    syslog(LOG_WARNING, "%s: invalid txlen %u", __func__, txlen);
    return -1;
  }

  tbuf[0] = (bus << 1) + 1;
  tbuf[1] = addr;
  tbuf[2] = rxlen;
  memcpy(&tbuf[3], txbuf, txlen);
  return bic_ipmb_wrapper(NETFN_APP_REQ, CMD_APP_MASTER_WRITE_READ,
                       tbuf, txlen+3, rxbuf, &rxlen);
}

struct vr_ops rns_ops = {
  .get_fw_ver = get_raa_ver,
  .parse_file = raa_parse_file,
  .validate_file = NULL,
  .fw_update = raa_fw_update,
  .fw_verify = NULL,
};

struct vr_ops ifx_ops = {
  .get_fw_ver = get_xdpe152xx_ver,
  .parse_file = xdpe152xx_parse_file,
  .validate_file = NULL,
  .fw_update = xdpe152xx_fw_update,
  .fw_verify = NULL,
};

struct vr_ops ti_ops = {
  .get_fw_ver = get_tps_ver,
  .parse_file = tps_parse_file,
  .validate_file = NULL,
  .fw_update = tps_fw_update,
  .fw_verify = NULL,
};

struct vr_info fbwc_vr_list[] = {
  {
    .bus = MB_BIC_VR_BUS,
    .addr = RNS_VCCIN_ADDR,
    .dev_name = "VCCIN/VCCFA_EHV_FIVRA",
    .ops = &rns_ops,
    .private_data = fru_name,
    .xfer = &fbwc_vr_rdwr,
  },
  {
    .bus = MB_BIC_VR_BUS,
    .addr = RNS_VCCD_ADDR,
    .dev_name = "VCCD",
    .ops = &rns_ops,
    .private_data = fru_name,
    .xfer = &fbwc_vr_rdwr,
  },
  {
    .bus = MB_BIC_VR_BUS,
    .addr = RNS_VCCINFAON_ADDR,
    .dev_name = "VCCINFAON/VCCFA_EHV",
    .ops = &rns_ops,
    .private_data = fru_name,
    .xfer = &fbwc_vr_rdwr,
  },
};

void plat_vr_preinit(const char *name) {
  if (name) {
    snprintf(fru_name, sizeof(fru_name), "%s", name);
  } else {
    fru_name[0] = 0;
  }
}

int plat_vr_init(void) {
  uint8_t rbuf[16], rlen;
  int i, vr_cnt = sizeof(fbwc_vr_list)/sizeof(fbwc_vr_list[0]);

  for (i = 0; i < vr_cnt; i++) {
    rlen = sizeof(rbuf);
    if (bic_bypass_get_vr_device_id(FRU_MB, rbuf, &rlen, fbwc_vr_list[i].bus,
                             fbwc_vr_list[i].addr) < 0) {
      continue;
    }

    switch (rlen) {
      case 2:
        fbwc_vr_list[i].ops = &ifx_ops;
        break;
      case 6:
        fbwc_vr_list[i].ops = &ti_ops;
        break;
      default:
        fbwc_vr_list[i].ops = &rns_ops;
        break;
    }
  }

  return vr_device_register(fbwc_vr_list, vr_cnt);
}

void plat_vr_exit(void) {
  if (plat_configs) {
    free(plat_configs);
  }

  return;
}
