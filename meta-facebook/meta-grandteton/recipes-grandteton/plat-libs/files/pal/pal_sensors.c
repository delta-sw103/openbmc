#include <stdio.h>
#include <syslog.h>
#include <openbmc/kv.h>
#include <openbmc/libgpio.h>
#include <openbmc/obmc-i2c.h>
#include <openbmc/obmc-sensors.h>
#include "pal.h"

//#define DEBUG
#define PECI_MUX_SELECT_BMC (GPIO_VALUE_HIGH)
#define PECI_MUX_SELECT_PCH (GPIO_VALUE_LOW)
#define SENSOR_SKIP_MAX (1)

extern PAL_SENSOR_MAP mb_sensor_map[];
extern PAL_SENSOR_MAP hmc_sensor_map[];
extern PAL_SENSOR_MAP swb_sensor_map[];
extern PAL_SENSOR_MAP bb_sensor_map[];

extern const uint8_t mb_sensor_list[];
extern const uint8_t swb_sensor_list[];
extern const uint8_t hmc_sensor_list[];
extern const uint8_t nic0_sensor_list[];
extern const uint8_t nic1_sensor_list[];
extern const uint8_t mb_discrete_sensor_list[];
extern const uint8_t vpdb_sensor_list[];
extern const uint8_t hpdb_sensor_list[];
extern const uint8_t bp0_sensor_list[];
extern const uint8_t bp1_sensor_list[];
extern const uint8_t scm_sensor_list[];

extern size_t mb_sensor_cnt;
extern size_t swb_sensor_cnt;
extern size_t hmc_sensor_cnt;
extern size_t mb_discrete_sensor_cnt;
extern size_t nic0_sensor_cnt;
extern size_t nic1_sensor_cnt;
extern size_t vpdb_sensor_cnt;
extern size_t hpdb_sensor_cnt;
extern size_t bp0_sensor_cnt;
extern size_t bp1_sensor_cnt;
extern size_t scm_sensor_cnt;


struct snr_map sensor_map[] = {
  { FRU_ALL, NULL},
  { FRU_MB,   mb_sensor_map },
  { FRU_SWB,  swb_sensor_map },
  { FRU_HMC,  hmc_sensor_map },
  { FRU_NIC0, bb_sensor_map },
  { FRU_NIC1, bb_sensor_map },
  { FRU_DBG,  NULL },
  { FRU_BMC,  NULL },
  { FRU_SCM,  bb_sensor_map },
  { FRU_PDBV, bb_sensor_map },
  { FRU_PDBH, bb_sensor_map },
  { FRU_BP0,  bb_sensor_map },
  { FRU_BP1,  bb_sensor_map },
};
int
pal_get_fru_sensor_list(uint8_t fru, uint8_t **sensor_list, int *cnt) {
  int ret=0;

  if (fru == FRU_MB) {
    *sensor_list = (uint8_t *) mb_sensor_list;
    *cnt = mb_sensor_cnt;
  } else if (fru == FRU_NIC0) {
    *sensor_list = (uint8_t *) nic0_sensor_list;
    *cnt = nic0_sensor_cnt;
  } else if (fru == FRU_NIC1) {
    *sensor_list = (uint8_t *) nic1_sensor_list;
    *cnt = nic1_sensor_cnt;
  } else if (fru == FRU_HMC) {
    *sensor_list = (uint8_t *) hmc_sensor_list;
    *cnt = hmc_sensor_cnt;
  } else if (fru == FRU_PDBV) {
    *sensor_list = (uint8_t *) vpdb_sensor_list;
    *cnt = vpdb_sensor_cnt;
  } else if (fru == FRU_PDBH) {
    *sensor_list = (uint8_t *) hpdb_sensor_list;
    *cnt = hpdb_sensor_cnt;
  } else if (fru == FRU_BP0) {
    *sensor_list = (uint8_t *) bp0_sensor_list;
    *cnt = bp0_sensor_cnt;
  } else if (fru == FRU_BP1) {
    *sensor_list = (uint8_t *) bp1_sensor_list;
    *cnt = bp1_sensor_cnt;
  } else if (fru == FRU_SCM) {
    *sensor_list = (uint8_t *) scm_sensor_list;
    *cnt = scm_sensor_cnt;
  } else if (fru == FRU_SWB) {
    *sensor_list = (uint8_t *) swb_sensor_list;
    *cnt = swb_sensor_cnt;
  } else if (fru > MAX_NUM_FRUS) {
    return -1;
  } else {
    *sensor_list = NULL;
    *cnt = 0;
  }
  return ret;
}

int
pal_get_fru_discrete_list(uint8_t fru, uint8_t **sensor_list, int *cnt) {
  if (fru == FRU_MB) {
    *sensor_list = (uint8_t *) mb_discrete_sensor_list;
    *cnt = mb_discrete_sensor_cnt;
  } else if (fru > MAX_NUM_FRUS) {
      return -1;
  } else {
    // Nothing to read yet.
    *sensor_list = NULL;
    *cnt = 0;
  }
  return 0;
}


int retry_err_handle(uint8_t retry_curr, uint8_t retry_max) {

  if( retry_curr <= retry_max) {
    return READING_SKIP;
  }
  return READING_NA;
}

int retry_skip_handle(uint8_t retry_curr, uint8_t retry_max) {

  if( retry_curr <= retry_max) {
    return READING_SKIP;
  }
  return 0;
}

static uint8_t*
get_map_retry(uint8_t fru)
{
  static uint8_t mb_retry[256] = {0};
  static uint8_t swb_retry[256] = {0};
  static uint8_t bb_retry[256] = {0};

  if (fru == FRU_SWB)
    return swb_retry;
  else if (fru == FRU_MB)
    return mb_retry;
  else
    return bb_retry;
}

int
pal_sensor_read_raw(uint8_t fru, uint8_t sensor_num, void *value) {
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};
  char fru_name[32];
  int ret=0;
  bool server_off;
  uint8_t *retry = get_map_retry(fru);

  pal_get_fru_name(fru, fru_name);
  sprintf(key, "%s_sensor%d", fru_name, sensor_num);

  server_off = is_server_off();
  if (fru == FRU_MB || fru == FRU_SCM ||
      fru == FRU_NIC0 || fru == FRU_NIC1 ||
      fru == FRU_PDBV || fru == FRU_PDBH ||
      fru == FRU_BP0 || fru == FRU_BP1 ||
      fru == FRU_SWB || fru == FRU_HMC) {
    if (server_off) {
      if (sensor_map[fru].map[sensor_num].stby_read == true) {
        ret = sensor_map[fru].map[sensor_num].read_sensor(fru, sensor_num, (float*) value);
      } else {
        ret = READING_NA;
      }
    } else {
      ret = sensor_map[fru].map[sensor_num].read_sensor(fru, sensor_num, (float*) value);
    }

    if ( ret == 0 ) {
      if( (sensor_map[fru].map[sensor_num].snr_thresh.ucr_thresh <= *(float*)value) &&
          (sensor_map[fru].map[sensor_num].snr_thresh.ucr_thresh != 0) ) {
        ret = retry_skip_handle(retry[sensor_num], SENSOR_SKIP_MAX);
        if ( ret == READING_SKIP ) {
          retry[sensor_num]++;
#ifdef DEBUG
          syslog(LOG_CRIT,"sensor retry=%d touch ucr thres=%f snrnum=0x%x value=%f\n",
                 retry[sensor_num],
                 sensor_map[fru].map[sensor_num].snr_thresh.ucr_thresh,
                 sensor_num,
                  *(float*)value );
#endif
        }

      } else if( (sensor_map[fru].map[sensor_num].snr_thresh.lcr_thresh >= *(float*)value) &&
                 (sensor_map[fru].map[sensor_num].snr_thresh.lcr_thresh != 0) ) {
        ret = retry_skip_handle(retry[sensor_num], SENSOR_SKIP_MAX);
        if ( ret == READING_SKIP ) {
          retry[sensor_num]++;
#ifdef DEBUG
          syslog(LOG_CRIT,"sensor retry=%d touch lcr thres=%f snrnum=0x%x value=%f\n",
                 retry[sensor_num],
                 sensor_map[fru].map[sensor_num].snr_thresh.lcr_thresh,
                 sensor_num,
                 *(float*)value );
#endif
        }
      } else {
        retry[sensor_num] = 0;
      }
    }
  } else {
    return -1;
  }

  if (ret) {
    if (ret == READING_NA || ret == -1) {
      strcpy(str, "NA");
    } else {
      return ret;
    }
  } else {
    sprintf(str, "%.2f",*((float*)value));
  }
  if (kv_set(key, str, 0, 0) < 0) {
    syslog(LOG_WARNING, "pal_sensor_read_raw: cache_set key = %s, str = %s failed.", key, str);
    return -1;
  } else {
    return ret;
  }

  return 0;
}

int
pal_get_sensor_name(uint8_t fru, uint8_t sensor_num, char *name) {
  char fru_name[32];
  char units_name[8];
  uint8_t scale = sensor_map[fru].map[sensor_num].units;


  if (fru == FRU_MB || fru == FRU_SCM ||
      fru == FRU_NIC0 || fru == FRU_NIC1 ||
      fru == FRU_PDBV || fru == FRU_PDBH ||
      fru == FRU_BP0 || fru == FRU_BP1 ||
      fru == FRU_SWB || fru == FRU_HMC) {

    pal_get_fru_name(fru, fru_name);
    if (fru_name != NULL)
      for (int i = 0; i < strlen(fru_name); i++)
        fru_name[i] = toupper(fru_name[i]);


    switch(scale) {
      case TEMP:
        sprintf(units_name, "_C");
        break;
      case FAN:
        sprintf(units_name, "_RPM");
        break;
      case VOLT:
        sprintf(units_name, "_V");
        break;
      case CURR:
        sprintf(units_name, "_A");
        break;
      case POWER:
        sprintf(units_name, "_W");
        break;
    }

    sprintf(name, "%s_%s%s", fru_name,
                             sensor_map[fru].map[sensor_num].snr_name,
                             units_name);
  } else {
    return -1;
  }
  return 0;
}

int
pal_get_sensor_threshold(uint8_t fru, uint8_t sensor_num, uint8_t thresh, void *value) {
  float *val = (float*) value;

  if (fru != FRU_MB && fru != FRU_SCM &&
      fru != FRU_NIC0 && fru != FRU_NIC1 &&
      fru != FRU_PDBV && fru != FRU_PDBH &&
      fru != FRU_BP0 && fru != FRU_BP1 &&
      fru != FRU_SWB && fru != FRU_HMC) {
    syslog(LOG_WARNING, "Threshold type error value=%d\n", thresh);
    return -1;
  }

  switch(thresh) {
    case UCR_THRESH:
      *val = sensor_map[fru].map[sensor_num].snr_thresh.ucr_thresh;
      break;
    case UNC_THRESH:
      *val = sensor_map[fru].map[sensor_num].snr_thresh.unc_thresh;
      break;
    case UNR_THRESH:
      *val = sensor_map[fru].map[sensor_num].snr_thresh.unr_thresh;
      break;
    case LCR_THRESH:
      *val = sensor_map[fru].map[sensor_num].snr_thresh.lcr_thresh;
      break;
    case LNC_THRESH:
      *val = sensor_map[fru].map[sensor_num].snr_thresh.lnc_thresh;
      break;
    case LNR_THRESH:
      *val = sensor_map[fru].map[sensor_num].snr_thresh.lnr_thresh;
      break;
    case POS_HYST:
      *val = sensor_map[fru].map[sensor_num].snr_thresh.pos_hyst;
      break;
    case NEG_HYST:
      *val = sensor_map[fru].map[sensor_num].snr_thresh.neg_hyst;
      break;
    default:
      return -1;
  }
  return 0;
}

int
pal_get_sensor_units(uint8_t fru, uint8_t sensor_num, char *units) {

  uint8_t scale = sensor_map[fru].map[sensor_num].units;

  if (fru != FRU_MB && fru != FRU_SCM &&
      fru != FRU_NIC0 && fru != FRU_NIC1 &&
      fru != FRU_PDBV && fru != FRU_PDBH &&
      fru != FRU_BP0 && fru != FRU_BP1 &&
      fru != FRU_SWB && fru != FRU_HMC) {
    return -1;
  }

  switch(scale) {
    case TEMP:
      sprintf(units, "C");
      break;
    case FAN:
      sprintf(units, "RPM");
      break;
    case VOLT:
      sprintf(units, "Volts");
      break;
    case CURR:
      sprintf(units, "Amps");
      break;
    case POWER:
      sprintf(units, "Watts");
      break;
    default:
      return -1;
  }
  return 0;
}

static int
gt_sensor_name(uint8_t fru, uint8_t sensor_num, char *name) {

  return -1;
}

int
pal_get_event_sensor_name(uint8_t fru, uint8_t *sel, char *name) {
  uint8_t snr_type = sel[10];
  uint8_t snr_num = sel[11];

  // If SNR_TYPE is OS_BOOT, sensor name is OS
  switch (snr_type) {
    case OS_BOOT:
      // OS_BOOT used by OS
      sprintf(name, "OS");
      return 0;
    default:
      if (gt_sensor_name(fru, snr_num, name) != 0) {
        break;
      }
      return 0;
  }

  // Otherwise, translate it based on snr_num
  return pal_get_x86_event_sensor_name(fru, snr_num, name);
}

int
pal_parse_sel(uint8_t fru, uint8_t *sel, char *error_log)
{
  uint8_t snr_num = sel[11];
  //uint8_t *event_data = &sel[10];
  //uint8_t *ed = &event_data[3];
  bool parsed = true;

  strcpy(error_log, "");
  switch(snr_num) {
    default:
      parsed = false;
      break;
  }

  if (parsed == true) {
    return 0;
  }

  pal_parse_sel_helper(fru, sel, error_log);
  return 0;
}

static int pal_set_peci_mux(uint8_t select) {
  gpio_desc_t *desc;
  bool ret = false;

  desc = gpio_open_by_shadow("PECI_MUX_SELECT");
  if (!desc)
    return ret;

  gpio_set_value(desc, select);
  gpio_close(desc);
  return 0;
}

void
pal_sensor_assert_handle(uint8_t fru, uint8_t snr_num, float val, uint8_t thresh) {
  char cmd[128];
  char sensor_name[32];
  char thresh_name[10];
  uint8_t fan_id;
  uint8_t cpu_id;

  switch (thresh) {
    case UNR_THRESH:
        sprintf(thresh_name, "UNR");
      break;
    case UCR_THRESH:
        sprintf(thresh_name, "UCR");
      break;
    case UNC_THRESH:
        sprintf(thresh_name, "UNCR");
      break;
    case LNR_THRESH:
        sprintf(thresh_name, "LNR");
      break;
    case LCR_THRESH:
        sprintf(thresh_name, "LCR");
      break;
    case LNC_THRESH:
        sprintf(thresh_name, "LNCR");
      break;
    default:
      syslog(LOG_WARNING, "%s: wrong thresh enum value", __func__);
      exit(-1);
  }

  switch(snr_num) {
    case MB_SNR_CPU0_TEMP:
    case MB_SNR_CPU1_TEMP:
      cpu_id = snr_num - MB_SNR_CPU0_TEMP;
      sprintf(cmd, "P%d Temp %s %3.0fC - Assert",cpu_id, thresh_name, val);
      break;
    case SCM_SNR_P12V:
    case SCM_SNR_P5V:
    case SCM_SNR_P3V3:
    case SCM_SNR_P2V5:
    case SCM_SNR_P1V8:
    case SCM_SNR_PGPPA:
    case SCM_SNR_P1V2:
    case SCM_SNR_P1V0:
    case MB_SNR_ADC128_P12V_AUX:
    case NB_SNR_ADC128_P5V:
    case MB_SNR_ADC128_P3V3:
    case MB_SNR_ADC128_P3V3_AUX:
    case MB_SNR_E1S_P12V_IOUT:
    case MB_SNR_P3V_BAT:
    case MB_SNR_HSC_VIN:
    case MB_SNR_VR_CPU0_VCCIN_VOLT:
    case MB_SNR_VR_CPU0_VCCFA_FIVRA_VOLT:
    case MB_SNR_VR_CPU0_VCCIN_FAON_VOLT:
    case MB_SNR_VR_CPU0_VCCFA_VOLT:
    case MB_SNR_VR_CPU0_VCCD_HV_VOLT:
    case MB_SNR_VR_CPU1_VCCIN_VOLT:
    case MB_SNR_VR_CPU1_VCCFA_FIVRA_VOLT:
    case MB_SNR_VR_CPU1_VCCIN_FAON_VOLT:
    case MB_SNR_VR_CPU1_VCCFA_VOLT:
    case MB_SNR_VR_CPU1_VCCD_HV_VOLT:
      pal_get_sensor_name(fru, snr_num, sensor_name);
      sprintf(cmd, "%s %s %.2fVolts - Assert", sensor_name, thresh_name, val);
      break;
    case BP0_SNR_FAN0_INLET_SPEED:
    case BP0_SNR_FAN1_INLET_SPEED:
    case BP1_SNR_FAN2_INLET_SPEED:
    case BP1_SNR_FAN3_INLET_SPEED:
    case BP0_SNR_FAN4_INLET_SPEED:
    case BP0_SNR_FAN5_INLET_SPEED:
    case BP1_SNR_FAN6_INLET_SPEED:
    case BP1_SNR_FAN7_INLET_SPEED:
    case BP0_SNR_FAN8_INLET_SPEED:
    case BP0_SNR_FAN9_INLET_SPEED:
    case BP1_SNR_FAN10_INLET_SPEED:
    case BP1_SNR_FAN11_INLET_SPEED:
    case BP0_SNR_FAN12_INLET_SPEED:
    case BP0_SNR_FAN13_INLET_SPEED:
    case BP1_SNR_FAN14_INLET_SPEED:
    case BP1_SNR_FAN15_INLET_SPEED:
      fan_id = snr_num-FAN_SNR_START_INDEX;
      sprintf(cmd, "FAN%d %s %dRPM - Assert",fan_id ,thresh_name, (int)val);
      break;
    case BP0_SNR_FAN0_OUTLET_SPEED:
    case BP0_SNR_FAN1_OUTLET_SPEED:
    case BP1_SNR_FAN2_OUTLET_SPEED:
    case BP1_SNR_FAN3_OUTLET_SPEED:
    case BP0_SNR_FAN4_OUTLET_SPEED:
    case BP0_SNR_FAN5_OUTLET_SPEED:
    case BP1_SNR_FAN6_OUTLET_SPEED:
    case BP1_SNR_FAN7_OUTLET_SPEED:
    case BP0_SNR_FAN8_OUTLET_SPEED:
    case BP0_SNR_FAN9_OUTLET_SPEED:
    case BP1_SNR_FAN10_OUTLET_SPEED:
    case BP1_SNR_FAN11_OUTLET_SPEED:
    case BP0_SNR_FAN12_OUTLET_SPEED:
    case BP0_SNR_FAN13_OUTLET_SPEED:
    case BP1_SNR_FAN14_OUTLET_SPEED:
    case BP1_SNR_FAN15_OUTLET_SPEED:
      fan_id = snr_num-FAN_SNR_START_INDEX;
      sprintf(cmd, "FAN%d %s %dRPM - Assert",fan_id ,thresh_name, (int)val);
      break;
    default:
      return;
  }
  pal_add_cri_sel(cmd);
}

void
pal_sensor_deassert_handle(uint8_t fru, uint8_t snr_num, float val, uint8_t thresh) {
  char cmd[128];
  char sensor_name[32];
  char thresh_name[10];
  uint8_t fan_id;
  uint8_t cpu_id;

  switch (thresh) {
    case UNR_THRESH:
        sprintf(thresh_name, "UNR");
      break;
    case UCR_THRESH:
        sprintf(thresh_name, "UCR");
      break;
    case UNC_THRESH:
        sprintf(thresh_name, "UNCR");
      break;
    case LNR_THRESH:
        sprintf(thresh_name, "LNR");
      break;
    case LCR_THRESH:
        sprintf(thresh_name, "LCR");
      break;
    case LNC_THRESH:
        sprintf(thresh_name, "LNCR");
      break;
    default:
      syslog(LOG_WARNING, "%s: wrong thresh enum value", __func__);
      exit(-1);
  }

  switch(snr_num) {
    case MB_SNR_CPU0_TEMP:
    case MB_SNR_CPU1_TEMP:
      cpu_id = snr_num - MB_SNR_CPU0_TEMP;
      sprintf(cmd, "P%d Temp %s %3.0fC - Deassert",cpu_id, thresh_name, val);
      break;
    case SCM_SNR_P12V:
    case SCM_SNR_P5V:
    case SCM_SNR_P3V3:
    case SCM_SNR_P2V5:
    case SCM_SNR_P1V8:
    case SCM_SNR_PGPPA:
    case SCM_SNR_P1V2:
    case SCM_SNR_P1V0:
    case MB_SNR_P3V_BAT:
    case MB_SNR_ADC128_P12V_AUX:
    case NB_SNR_ADC128_P5V:
    case MB_SNR_ADC128_P3V3:
    case MB_SNR_ADC128_P3V3_AUX:
    case MB_SNR_E1S_P12V_IOUT:
    case MB_SNR_HSC_VIN:
    case MB_SNR_VR_CPU0_VCCIN_VOLT:
    case MB_SNR_VR_CPU0_VCCFA_FIVRA_VOLT:
    case MB_SNR_VR_CPU0_VCCIN_FAON_VOLT:
    case MB_SNR_VR_CPU0_VCCFA_VOLT:
    case MB_SNR_VR_CPU0_VCCD_HV_VOLT:
    case MB_SNR_VR_CPU1_VCCIN_VOLT:
    case MB_SNR_VR_CPU1_VCCFA_FIVRA_VOLT:
    case MB_SNR_VR_CPU1_VCCIN_FAON_VOLT:
    case MB_SNR_VR_CPU1_VCCFA_VOLT:
    case MB_SNR_VR_CPU1_VCCD_HV_VOLT:
      pal_get_sensor_name(fru, snr_num, sensor_name);
      sprintf(cmd, "%s %s %.2fVolts - Deassert", sensor_name, thresh_name, val);
      break;
    case BP0_SNR_FAN0_INLET_SPEED:
    case BP0_SNR_FAN1_INLET_SPEED:
    case BP1_SNR_FAN2_INLET_SPEED:
    case BP1_SNR_FAN3_INLET_SPEED:
    case BP0_SNR_FAN4_INLET_SPEED:
    case BP0_SNR_FAN5_INLET_SPEED:
    case BP1_SNR_FAN6_INLET_SPEED:
    case BP1_SNR_FAN7_INLET_SPEED:
    case BP0_SNR_FAN8_INLET_SPEED:
    case BP0_SNR_FAN9_INLET_SPEED:
    case BP1_SNR_FAN10_INLET_SPEED:
    case BP1_SNR_FAN11_INLET_SPEED:
    case BP0_SNR_FAN12_INLET_SPEED:
    case BP0_SNR_FAN13_INLET_SPEED:
    case BP1_SNR_FAN14_INLET_SPEED:
    case BP1_SNR_FAN15_INLET_SPEED:
      fan_id= snr_num-FAN_SNR_START_INDEX;
      sprintf(cmd, "FAN%d %s %dRPM - Deassert",fan_id ,thresh_name, (int)val);
      break;
    case BP0_SNR_FAN0_OUTLET_SPEED:
    case BP0_SNR_FAN1_OUTLET_SPEED:
    case BP1_SNR_FAN2_OUTLET_SPEED:
    case BP1_SNR_FAN3_OUTLET_SPEED:
    case BP0_SNR_FAN4_OUTLET_SPEED:
    case BP0_SNR_FAN5_OUTLET_SPEED:
    case BP1_SNR_FAN6_OUTLET_SPEED:
    case BP1_SNR_FAN7_OUTLET_SPEED:
    case BP0_SNR_FAN8_OUTLET_SPEED:
    case BP0_SNR_FAN9_OUTLET_SPEED:
    case BP1_SNR_FAN10_OUTLET_SPEED:
    case BP1_SNR_FAN11_OUTLET_SPEED:
    case BP0_SNR_FAN12_OUTLET_SPEED:
    case BP0_SNR_FAN13_OUTLET_SPEED:
    case BP1_SNR_FAN14_OUTLET_SPEED:
    case BP1_SNR_FAN15_OUTLET_SPEED:
      fan_id = snr_num-FAN_SNR_START_INDEX;
      sprintf(cmd, "FAN%d %s %dRPM - Deassert",fan_id ,thresh_name, (int)val);
      break;
    default:
      return;
  }
  pal_add_cri_sel(cmd);

}

int pal_sensor_monitor_initial(void) {
//Initial
  syslog(LOG_DEBUG,"Sensor Initial\n");

//Config PECI Switch
  pal_set_peci_mux(PECI_MUX_SELECT_BMC);
  return 0;
}
