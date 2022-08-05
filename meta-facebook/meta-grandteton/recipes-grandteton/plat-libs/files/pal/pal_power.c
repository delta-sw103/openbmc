#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <syslog.h>
#include <pthread.h>
#include <openbmc/kv.h>
#include <openbmc/obmc-pal.h>
#include <openbmc/libgpio.h>
#include <openbmc/nm.h>
#include "pal_gpio.h"
#include "pal_def.h"
#include "pal_common.h"
#include "pal_power.h"


#define DELAY_POWER_ON 1
#define DELAY_POWER_OFF 6
#define DELAY_GRACEFUL_SHUTDOWN 1
#define DELAY_POWER_CYCLE 10

static bool m_chassis_ctrl = false;

bool
is_server_off(void) {
  int ret;
  uint8_t status;

  ret = pal_get_server_power(FRU_MB, &status);
  if (ret) {
    return false;
  }

  return status == SERVER_POWER_OFF? true: false;
}

static int
power_btn_out_pulse(uint8_t secs) {
  int ret = 0;
  gpio_desc_t *gdesc;

  gdesc = gpio_open_by_shadow(FP_PWR_BTN_OUT_N);
  if (!gdesc)
    return -1;

  do {
    ret = gpio_set_value(gdesc, GPIO_VALUE_HIGH);
    if (ret)
      break;

    ret = gpio_set_value(gdesc, GPIO_VALUE_LOW);
    if (ret)
      break;

    sleep(secs);

    ret = gpio_set_value(gdesc, GPIO_VALUE_HIGH);
  } while (0);

  gpio_close(gdesc);
  return ret;
}

// Power Off the server
static int
server_power_off(bool gs_flag) {
  return power_btn_out_pulse((gs_flag) ? DELAY_GRACEFUL_SHUTDOWN : DELAY_POWER_OFF);
}

// Power On the server
static int
server_power_on(void) {
  return power_btn_out_pulse(DELAY_POWER_ON);
}

int
pal_power_button_override(uint8_t fru_id) {
  return power_btn_out_pulse(DELAY_POWER_OFF);
}

int
pal_get_server_power(uint8_t fru, uint8_t *status) {
  int ret;
  gpio_desc_t *gdesc = NULL;
  gpio_value_t val;

  if (fru != FRU_MB)
    return -1;

  gdesc = gpio_open_by_shadow(FM_LAST_PWRGD);
  if (gdesc == NULL)
    return -1;

  ret = gpio_get_value(gdesc, &val);
  if (ret != 0)
    goto error;

  *status = (int)val;
error:
  gpio_close(gdesc);
  return ret;
}

// Power Off, Power On, or Power Reset the server in given slot
int
pal_set_server_power(uint8_t fru, uint8_t cmd) {
  uint8_t status;
  bool gs_flag = false;
  int ret;

  if (pal_get_server_power(fru, &status) < 0) {
    return -1;
  }

  switch(cmd) {
    case SERVER_POWER_ON:
      if (status == SERVER_POWER_ON)
        return 1;
      else
        return server_power_on();
      break;

    case SERVER_POWER_OFF:
      if (status == SERVER_POWER_OFF)
        return 1;
      else
        return server_power_off(gs_flag);
      break;

    case SERVER_POWER_CYCLE:
      if (status == SERVER_POWER_ON) {
        if (server_power_off(gs_flag))
          return -1;

        sleep(DELAY_POWER_CYCLE);

        return server_power_on();

      } else if (status == SERVER_POWER_OFF) {

        return (server_power_on());
      }
      break;

    case SERVER_GRACEFUL_SHUTDOWN:
      if (status == SERVER_POWER_OFF)
        return 1;
      gs_flag = true;
      return server_power_off(gs_flag);
      break;

   case SERVER_POWER_RESET:
      if (status == SERVER_POWER_ON) {
        ret = pal_set_rst_btn(fru, 0);
        msleep(100); //some server miss to detect a quick pulse, so delay 100ms between low high
        ret |= pal_set_rst_btn(fru, 1);
        if (ret)
          return ret;
      } else if (status == SERVER_POWER_OFF)
        return -1;
      break;

    default:
      return -1;
  }

  return 0;
}

int set_me_entry_into_recovery(void) {
  int ret=-1;
  uint8_t retry = 3;
  NM_RW_INFO info;
  uint8_t rbuf[32];

  if (pal_skip_access_me()) {
    syslog(LOG_WARNING, "[%s] skip access NM", __func__);
    return 0;
  }

  // Send Command to ME into recovery mode;
  info.bus = NM_IPMB_BUS_ID;
  info.nm_addr = NM_SLAVE_ADDR;
  info.bmc_addr = BMC_DEF_SLAVE_ADDR;

#ifdef DEBUG
  syslog(LOG_CRIT, "nm_bus=%d, nm_addr=0x%x, bmc_addr=0x%x\n",
  info.bus, info.nm_addr, info.bmc_addr);
#endif

  while ( (retry > 0) ) {
    ret = cmd_NM_set_me_entry_recovery(info, rbuf);

    if ( ret != 0 )
      retry--;
    else
      break;
  }
  return ret;
}

//Systm AC Cycle
int pal_sled_cycle(void) {
  return system("i2cset -f -y 38 0x10 0xd9 c &> /dev/null");	
}

// Return the front panel's Reset Button status
int
pal_get_rst_btn(uint8_t *status) {
  int ret = -1;
  gpio_value_t value;
  gpio_desc_t *desc = gpio_open_by_shadow(FP_RST_BTN_IN_N);
  if (!desc) {
    return -1;
  }

  ret = gpio_get_value(desc, &value);
  if ( ret == 0 ) {
    *status = (value == GPIO_VALUE_HIGH ? 0 : 1);
  }
  gpio_close(desc);
  return ret;
}

int
pal_set_rst_btn(uint8_t slot, uint8_t status) {
  int ret;
  gpio_desc_t *gdesc = NULL;
  gpio_value_t val;

  if (slot != FRU_MB) {
    return -1;
  }

  gdesc = gpio_open_by_shadow(FP_RST_BTN_OUT_N);
  if (gdesc == NULL)
    return -1;

  val = status? GPIO_VALUE_HIGH: GPIO_VALUE_LOW;
  ret = gpio_set_value(gdesc, val);
  if (ret != 0)
    goto error;

error:
  gpio_close(gdesc);
  return ret;
}

int
pal_set_last_pwr_state(uint8_t fru, char *state) {

  int ret;
  char key[MAX_KEY_LEN] = {0};

  sprintf(key, "%s", "pwr_server_last_state");

  ret = pal_set_key_value(key, state);
  if (ret < 0) {
#ifdef DEBUG
    syslog(LOG_WARNING, "pal_set_last_pwr_state: pal_set_key_value failed for "
        "fru %u", fru);
#endif
  }

  return ret;
}

int
pal_get_last_pwr_state(uint8_t fru, char *state) {
  int ret;
  char key[MAX_KEY_LEN] = {0};

  sprintf(key, "%s", "pwr_server_last_state");

  ret = pal_get_key_value(key, state);
  if (ret < 0) {
#ifdef DEBUG
    syslog(LOG_WARNING, "pal_get_last_pwr_state: pal_get_key_value failed for "
        "fru %u", fru);
#endif
  }

  return ret;
}

uint8_t
pal_set_power_restore_policy(uint8_t slot, uint8_t *pwr_policy, uint8_t *res_data) {
  int cc = CC_SUCCESS;  // Fill response with default values
//  uint8_t policy = *pwr_policy & 0x07;  // Power restore policy

  cc = (int)pal_set_slot_power_policy(pwr_policy, res_data);
  return cc;
}

uint8_t 
pal_set_slot_power_policy(uint8_t *pwr_policy, uint8_t *res_data)
{
  int cc = CC_SUCCESS;
  uint8_t policy = *pwr_policy & 0x07;  // Power restore policy

  switch (policy) {
    case 0:
      if (pal_set_key_value("server_por_cfg", "off") != 0)
        cc = CC_UNSPECIFIED_ERROR;
      break;
    case 1:
      if (pal_set_key_value("server_por_cfg", "lps") != 0)
        cc = CC_UNSPECIFIED_ERROR;
      break;
    case 2:
      if (pal_set_key_value("server_por_cfg", "on") != 0)
        cc = CC_UNSPECIFIED_ERROR;
      break;
    case 3:
      // no change (just get present policy support)
      break;
    default:
      cc = CC_PARAM_OUT_OF_RANGE;
      break;
  }
  return cc;
}

void
pal_set_def_restart_cause(uint8_t slot)
{
  char pwr_policy[MAX_VALUE_LEN] = {0};
  char last_pwr_st[MAX_VALUE_LEN] = {0};
  if ( FRU_MB == slot )
  {
    kv_get("pwr_server_last_state", last_pwr_st, NULL, KV_FPERSIST);
    kv_get("server_por_cfg", pwr_policy, NULL, KV_FPERSIST);
    if( pal_is_bmc_por() )
    {
      if( !strcmp( pwr_policy, "on") )
      {
        pal_set_restart_cause(FRU_MB, RESTART_CAUSE_AUTOMATIC_PWR_UP);
      }
      else if( !strcmp( pwr_policy, "lps") && !strcmp( last_pwr_st, "on") )
      {
        pal_set_restart_cause(FRU_MB, RESTART_CAUSE_AUTOMATIC_PWR_UP_LPR);
      }
    }
  }
}

static void *
chassis_ctrl_hndlr(void *arg) {
  int cmd = (int)arg;

  pthread_detach(pthread_self());
  msleep(500);

  if (!pal_set_server_power(FRU_MB, cmd)) {
    switch (cmd) {
      case SERVER_POWER_OFF:
        syslog(LOG_CRIT, "SERVER_POWER_OFF successful for FRU: %d", FRU_MB);
        break;
      case SERVER_POWER_ON:
        syslog(LOG_CRIT, "SERVER_POWER_ON successful for FRU: %d", FRU_MB);
        break;
      case SERVER_POWER_CYCLE:
        syslog(LOG_CRIT, "SERVER_POWER_CYCLE successful for FRU: %d", FRU_MB);
        break;
      case SERVER_POWER_RESET:
        syslog(LOG_CRIT, "SERVER_POWER_RESET successful for FRU: %d", FRU_MB);
        break;
      case SERVER_GRACEFUL_SHUTDOWN:
        syslog(LOG_CRIT, "SERVER_GRACEFUL_SHUTDOWN successful for FRU: %d", FRU_MB);
        break;
    }
  }

  m_chassis_ctrl = false;
  pthread_exit(0);
}

int
pal_chassis_control(uint8_t fru, uint8_t *req_data, uint8_t req_len) {
  uint8_t comp_code = CC_SUCCESS;
  int ret, cmd = 0xFF;
  pthread_t tid;

  if (req_len != 1) {
    return CC_INVALID_LENGTH;
  }

  if (m_chassis_ctrl != false) {
    return CC_NOT_SUPP_IN_CURR_STATE;
  }

  switch (req_data[0]) {
    case 0x00:  // power off
      cmd = SERVER_POWER_OFF;
      break;
    case 0x01:  // power on
      cmd = SERVER_POWER_ON;
      break;
    case 0x02:  // power cycle
      cmd = SERVER_POWER_CYCLE;
      break;
    case 0x03:  // power reset
      cmd = SERVER_POWER_RESET;
      break;
    case 0x05:  // graceful-shutdown
      cmd = SERVER_GRACEFUL_SHUTDOWN;
      break;
    default:
      comp_code = CC_INVALID_DATA_FIELD;
      break;
  }

  if (comp_code == CC_SUCCESS) {
    m_chassis_ctrl = true;
    ret = pthread_create(&tid, NULL, chassis_ctrl_hndlr, (void *)cmd);
    if (ret < 0) {
      syslog(LOG_WARNING, "[%s] Create chassis_ctrl_hndlr thread failed!", __func__);
      m_chassis_ctrl = false;
      return CC_NODE_BUSY;
    }
  }

  return comp_code;
}

int
pal_is_bmc_por(void) {
  FILE *fp;
  int por = 0;

  fp = fopen("/tmp/ast_por", "r");
  if (fp != NULL) {
    if (fscanf(fp, "%d", &por) != 1) {
      por = 0;
    }
    fclose(fp);
  }

  return (por)?1:0;
}

void
pal_get_chassis_status(uint8_t slot, uint8_t *req_data, uint8_t *res_data, uint8_t *res_len) {

   char str_server_por_cfg[64];
   char buff[MAX_VALUE_LEN];
   int policy = 3;
   unsigned char *data = res_data;

   // Platform Power Policy
   memset(str_server_por_cfg, 0 , sizeof(char) * 64);
   sprintf(str_server_por_cfg, "%s", "server_por_cfg");

   if (pal_get_key_value(str_server_por_cfg, buff) == 0)
   {
     if (!memcmp(buff, "off", strlen("off")))
       policy = 0;
     else if (!memcmp(buff, "lps", strlen("lps")))
       policy = 1;
     else if (!memcmp(buff, "on", strlen("on")))
       policy = 2;
     else
       policy = 3;
   }
   *data++ = ((is_server_off())?0x00:0x01) | (policy << 5);
   *data++ = 0x00;   // Last Power Event
   *data++ = 0x40;   // Misc. Chassis Status
   *data++ = 0x00;   // Front Panel Button Disable
   *res_len = data - res_data;
}
