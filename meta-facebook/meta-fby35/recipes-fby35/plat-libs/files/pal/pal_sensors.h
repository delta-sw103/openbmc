#ifndef __PAL_SENSORS_H__
#define __PAL_SENSORS_H__

#include <openbmc/obmc_pal_sensors.h>

#define MAX_SENSOR_NUM         (0xFF)
#define MAX_DEVICE_NAME_SIZE   (128)

//AMD1278 CMD INFO
#define PMBUS_PMON_CONFIG  (0xD4)
#define ADM1278_SLAVE_ADDR (0x80)
#define ADM1278_RSENSE     (0.5)
#define ADM1278_EIN_EXT    (0xDC)
#define ADM1278_PEAK_IOUT  (0xD0)
#define ADM1278_PEAK_PIN   (0xDA)

//PMBus
#define PMBUS_PAGE         (0x00)
#define PMBUS_VOUT_MODE    (0x20)
#define PMBUS_VOUT_COMMAND (0x21)
#define PMBUS_READ_VIN     (0x88)
#define PMBUS_READ_IIN     (0x89)
#define PMBUS_READ_VOUT    (0x8B)
#define PMBUS_READ_IOUT    (0x8C)
#define PMBUS_READ_TEMP1   (0x8D)
#define PMBUS_READ_TEMP2   (0x8E)
#define PMBUS_READ_POUT    (0x96)
#define PMBUS_READ_PIN     (0x97)

#define PREFIX_1OU_M2A 0x60
#define PREFIX_1OU_M2B 0x68
#define PREFIX_1OU_M2C 0x70
#define PREFIX_1OU_M2D 0x78

#define PREFIX_2OU_M2A 0x80
#define PREFIX_2OU_M2B 0x88
#define PREFIX_2OU_M2C 0xB0
#define PREFIX_2OU_M2D 0xB8
#define PREFIX_2OU_M2E 0xC0
#define PREFIX_2OU_M2F 0xC8

#define SUFFIX_INA231_PWR 0x00
#define SUFFIX_INA231_VOL 0x01

#define HSC_OUTPUT_CUR_UC_THRESHOLD 0xA7
#define HSC_INPUT_PWR_UC_THRESHOLD 0xD2

typedef struct {
  float ucr_thresh;
  float unc_thresh;
  float unr_thresh;
  float lcr_thresh;
  float lnc_thresh;
  float lnr_thresh;
  float pos_hyst;
  float neg_hyst;
} PAL_SENSOR_THRESHOLD;

typedef struct {
  char* snr_name;
  uint8_t id;
  int (*read_sensor) (uint8_t id, float *value);
  uint8_t stby_read;
  PAL_SENSOR_THRESHOLD snr_thresh;
  uint8_t units;
} PAL_SENSOR_MAP;

enum {
  UNSET_UNIT = 0,
  TEMP = 1,
  CURR,
  VOLT,
  FAN,
  PERCENT,
  POWER,
};

//Sensor Table
enum {
  //BIC - threshold sensors
  BIC_SENSOR_INLET_TEMP = 0x1,
  BIC_SENSOR_OUTLET_TEMP = 0x2,
  BIC_SENSOR_FIO_TEMP = 0x3,
  BIC_SENSOR_PCH_TEMP = 0x4,
  BIC_SENSOR_CPU_TEMP = 0x5,
  BIC_SENSOR_DIMMA0_TEMP = 0x6,
  BIC_SENSOR_DIMMB0_TEMP = 0x7,
  BIC_SENSOR_DIMMC0_TEMP = 0x9,
  BIC_SENSOR_DIMMD0_TEMP = 0xA,
  BIC_SENSOR_DIMME0_TEMP = 0xB,
  BIC_SENSOR_DIMMF0_TEMP = 0xC,
  BIC_SENSOR_CPU_THERM_MARGIN = 0xD,
  BIC_SENSOR_M2A_TEMP = 0x1F,
  BIC_SENSOR_M2B_TEMP = 0xE,
  BIC_SENSOR_HSC_TEMP = 0xF,
  BIC_SENSOR_VCCIN_VR_TEMP = 0x10,
  BIC_SENSOR_VCCSA_VR_TEMP = 0x11,
  BIC_SENSOR_VCCIO_VR_Temp = 0x12,
  BIC_SENSOR_P3V3_STBY_VR_TEMP = 0x13,
  BIC_SENSOR_PVDDQ_ABC_VR_TEMP = 0x14,
  BIC_SENSOR_PVDDQ_DEF_VR_TEMP = 0x15,

  //BIC - voltage sensors
  BIC_SENSOR_P12V_STBY_VOL = 0x20,
  BIC_SENSOR_P3V_BAT_VOL = 0x21,
  BIC_SENSOR_P3V3_STBY_VOL = 0x22,
  BIC_SENSOR_P1V05_PCH_STBY_VOL = 0x23,
  BIC_SENSOR_PVNN_PCH_STBY_VOL = 0x24,
  BIC_SENSOR_CPU_TJMAX = 0x25,
  BIC_SENSOR_HSC_INPUT_VOL = 0x26,
  BIC_SENSOR_VCCIN_VR_VOL = 0x27,
  BIC_SENSOR_VCCSA_VR_VOL = 0x28,
  BIC_SENSOR_VCCIO_VR_VOL =0x29,
  BIC_SENSOR_P3V3_STBY_VR_VOL = 0x2A,
  BIC_PVDDQ_ABC_VR_VOL = 0x2C,
  BIC_PVDDQ_DEF_VR_VOL = 0x2D,

  //BIC - current sensors
  BIC_SENSOR_HSC_OUTPUT_CUR = 0x30,
  BIC_SENSOR_VCCIN_VR_CUR = 0x31,
  BIC_SENSOR_VCCSA_VR_CUR = 0x32,
  BIC_SENSOR_VCCIO_VR_CUR = 0x33,
  BIC_SENSOR_P3V3_STBY_VR_CUR = 0x34,
  BIC_SENSOR_PVDDQ_ABC_VR_CUR = 0x35,
  BIC_SENSOR_PVDDQ_DEF_VR_CUR = 0x36,

  //BIC - power sensors
  BIC_SENSOR_HSC_INPUT_PWR = 0x2E,
  BIC_SENSOR_HSC_INPUT_AVGPWR = 0x39,
  BIC_SENSOR_VCCIN_VR_POUT = 0x3A,
  BIC_SENSOR_VCCSA_VR_POUT = 0x3C,
  BIC_SENSOR_VCCIO_VR_POUT = 0x3D,
  BIC_SENSOR_P3V3_STBY_VR_POUT = 0x3E,
  BIC_SENSOR_PVDDQ_ABC_VR_POUT = 0x3F,
  BIC_SENSOR_PVDDQ_DEF_VR_POUT = 0x42,
  BIC_SENSOR_SOC_PKG_PWR = 0x1E,

  //BIC 1OU EXP Sensors
  BIC_1OU_EXP_SENSOR_OUTLET_TEMP = 0x50,
  BIC_1OU_EXP_SENSOR_P12_VOL = 0x52,
  BIC_1OU_EXP_SENSOR_P3V3_VOL = 0x53,
  BIC_1OU_EXP_SENSOR_P1V8_VOL = 0x54,
  BIC_1OU_EXP_SENSOR_P3V3_STBY_VR_VOL = 0x55,
  BIC_1OU_EXP_SENSOR_P3V3_STBY2_VR_VOL = 0x57,
  BIC_1OU_EXP_SENSOR_P3V3_M2A_PWR = (PREFIX_1OU_M2A | SUFFIX_INA231_PWR),
  BIC_1OU_EXP_SENSOR_P3V3_M2A_VOL = (PREFIX_1OU_M2A | SUFFIX_INA231_VOL),
  BIC_1OU_EXP_SENSOR_P3V3_M2A_TMP = 0x62,
  BIC_1OU_EXP_SENSOR_P3V3_M2B_PWR = (PREFIX_1OU_M2B | SUFFIX_INA231_PWR),
  BIC_1OU_EXP_SENSOR_P3V3_M2B_VOL = (PREFIX_1OU_M2B | SUFFIX_INA231_VOL),
  BIC_1OU_EXP_SENSOR_P3V3_M2B_TMP = 0x6A,
  BIC_1OU_EXP_SENSOR_P3V3_M2C_PWR = (PREFIX_1OU_M2C | SUFFIX_INA231_PWR),
  BIC_1OU_EXP_SENSOR_P3V3_M2C_VOL = (PREFIX_1OU_M2C | SUFFIX_INA231_VOL),
  BIC_1OU_EXP_SENSOR_P3V3_M2C_TMP = 0x72,
  BIC_1OU_EXP_SENSOR_P3V3_M2D_PWR = (PREFIX_1OU_M2D | SUFFIX_INA231_PWR),
  BIC_1OU_EXP_SENSOR_P3V3_M2D_VOL = (PREFIX_1OU_M2D | SUFFIX_INA231_VOL),
  BIC_1OU_EXP_SENSOR_P3V3_M2D_TMP = 0x7A,

  //BIC 1OU EDSFF Sensors
  BIC_1OU_EDSFF_SENSOR_NUM_T_MB_OUTLET_TEMP_T = 0x50,
  BIC_1OU_EDSFF_SENSOR_NUM_V_12_AUX = 0x51,
  BIC_1OU_EDSFF_SENSOR_NUM_V_12_EDGE = 0x52,
  BIC_1OU_EDSFF_SENSOR_NUM_V_3_3_AUX = 0x53,
  BIC_1OU_EDSFF_SENSOR_NUM_V_HSC_IN = 0x54,
  BIC_1OU_EDSFF_SENSOR_NUM_I_HSC_OUT = 0x55,
  BIC_1OU_EDSFF_SENSOR_NUM_P_HSC_IN = 0x56,
  BIC_1OU_EDSFF_SENSOR_NUM_T_HSC = 0x57,
  BIC_1OU_EDSFF_SENSOR_NUM_INA231_PWR_M2A = 0x60,
  BIC_1OU_EDSFF_SENSOR_NUM_INA231_VOL_M2A = 0x61,
  BIC_1OU_EDSFF_SENSOR_NUM_NVME_TEMP_M2A = 0x62,
  BIC_1OU_EDSFF_SENSOR_NUM_ADC_12V_VOL_M2A = 0x63,
  BIC_1OU_EDSFF_SENSOR_NUM_ADC_3V3_VOL_M2A = 0x64,
  BIC_1OU_EDSFF_SENSOR_NUM_INA231_PWR_M2B = 0x68,
  BIC_1OU_EDSFF_SENSOR_NUM_INA231_VOL_M2B = 0x69,
  BIC_1OU_EDSFF_SENSOR_NUM_NVME_TEMP_M2B = 0x6A,
  BIC_1OU_EDSFF_SENSOR_NUM_ADC_12V_VOL_M2B = 0x6B,
  BIC_1OU_EDSFF_SENSOR_NUM_ADC_3V3_VOL_M2B = 0x6C,
  BIC_1OU_EDSFF_SENSOR_NUM_INA231_PWR_M2C = 0x70,
  BIC_1OU_EDSFF_SENSOR_NUM_INA231_VOL_M2C = 0x71,
  BIC_1OU_EDSFF_SENSOR_NUM_NVME_TEMP_M2C = 0x72,
  BIC_1OU_EDSFF_SENSOR_NUM_ADC_12V_VOL_M2C = 0x73,
  BIC_1OU_EDSFF_SENSOR_NUM_ADC_3V3_VOL_M2C = 0x74,
  BIC_1OU_EDSFF_SENSOR_NUM_INA231_PWR_M2D = 0x78,
  BIC_1OU_EDSFF_SENSOR_NUM_INA231_VOL_M2D = 0x79,
  BIC_1OU_EDSFF_SENSOR_NUM_NVME_TEMP_M2D = 0x7A,
  BIC_1OU_EDSFF_SENSOR_NUM_ADC_12V_VOL_M2D = 0x7B,
  BIC_1OU_EDSFF_SENSOR_NUM_ADC_3V3_VOL_M2D = 0x7C,

  //BIC 2OU EXP Sensors
  BIC_2OU_EXP_SENSOR_OUTLET_TEMP = 0x91,
  BIC_2OU_EXP_SENSOR_P12_VOL = 0x92,
  BIC_2OU_EXP_SENSOR_P3V3_VOL = 0x93,
  BIC_2OU_EXP_SENSOR_P1V8_VOL = 0x94,
  BIC_2OU_EXP_SENSOR_P3V3_STBY_VR_VOL = 0x95,
  BIC_2OU_EXP_SENSOR_P3V3_STBY2_VR_VOL = 0x96,
  BIC_2OU_EXP_SENSOR_P3V3_M2A_PWR = (PREFIX_2OU_M2A | SUFFIX_INA231_PWR),
  BIC_2OU_EXP_SENSOR_P3V3_M2A_VOL = (PREFIX_2OU_M2A | SUFFIX_INA231_VOL),
  BIC_2OU_EXP_SENSOR_P3V3_M2A_TMP = 0x82,
  BIC_2OU_EXP_SENSOR_P3V3_M2B_PWR = (PREFIX_2OU_M2B | SUFFIX_INA231_PWR),
  BIC_2OU_EXP_SENSOR_P3V3_M2B_VOL = (PREFIX_2OU_M2B | SUFFIX_INA231_VOL),
  BIC_2OU_EXP_SENSOR_P3V3_M2B_TMP = 0x8A,
  BIC_2OU_EXP_SENSOR_P3V3_M2C_PWR = (PREFIX_2OU_M2C | SUFFIX_INA231_PWR),
  BIC_2OU_EXP_SENSOR_P3V3_M2C_VOL = (PREFIX_2OU_M2C | SUFFIX_INA231_VOL),
  BIC_2OU_EXP_SENSOR_P3V3_M2C_TMP = 0xB2,
  BIC_2OU_EXP_SENSOR_P3V3_M2D_PWR = (PREFIX_2OU_M2D | SUFFIX_INA231_PWR),
  BIC_2OU_EXP_SENSOR_P3V3_M2D_VOL = (PREFIX_2OU_M2D | SUFFIX_INA231_VOL),
  BIC_2OU_EXP_SENSOR_P3V3_M2D_TMP = 0xBA,
  BIC_2OU_EXP_SENSOR_P3V3_M2E_PWR = (PREFIX_2OU_M2E | SUFFIX_INA231_PWR),
  BIC_2OU_EXP_SENSOR_P3V3_M2E_VOL = (PREFIX_2OU_M2E | SUFFIX_INA231_VOL),
  BIC_2OU_EXP_SENSOR_P3V3_M2E_TMP = 0xC2,
  BIC_2OU_EXP_SENSOR_P3V3_M2F_PWR = (PREFIX_2OU_M2F | SUFFIX_INA231_PWR),
  BIC_2OU_EXP_SENSOR_P3V3_M2F_VOL = (PREFIX_2OU_M2F | SUFFIX_INA231_VOL),
  BIC_2OU_EXP_SENSOR_P3V3_M2F_TMP = 0xCA,

  //BIC 2OU GPV3 sensors
  BIC_GPV3_INLET_TEMP = 0x91,
  BIC_GPV3_PCIE_SW_TEMP = 0x92,
  BIC_GPV3_P3V3_STBY1_TEMP = 0x93,
  BIC_GPV3_P3V3_STBY2_TEMP = 0x94,
  BIC_GPV3_P3V3_STBY3_TEMP = 0x95,
  BIC_GPV3_VR_P1V8_TEMP = 0x96,
  BIC_GPV3_VR_P0V84_TEMP = 0x97,
  BIC_GPV3_NVME_TEMP_DEV0 = 0x82,
  BIC_GPV3_NVME_TEMP_DEV1 = 0x86,
  BIC_GPV3_NVME_TEMP_DEV2 = 0x8A,
  BIC_GPV3_NVME_TEMP_DEV3 = 0x8E,
  BIC_GPV3_NVME_TEMP_DEV4 = 0xB2,
  BIC_GPV3_NVME_TEMP_DEV5 = 0xB6,
  BIC_GPV3_NVME_TEMP_DEV6 = 0xBA,
  BIC_GPV3_NVME_TEMP_DEV7 = 0xBE,
  BIC_GPV3_NVME_TEMP_DEV8 = 0xC2,
  BIC_GPV3_NVME_TEMP_DEV9 = 0xC6,
  BIC_GPV3_NVME_TEMP_DEV10 = 0xCA,
  BIC_GPV3_NVME_TEMP_DEV11 = 0xCE,
  BIC_GPV3_PESW_PWR = 0x8F,
  BIC_GPV3_E1S_1_TEMP = 0xA2,
  BIC_GPV3_E1S_2_TEMP = 0xA6,
  BIC_GPV3_P3V3_STBY1_VOLTAGE = 0x9A,
  BIC_GPV3_P3V3_STBY2_VOLTAGE = 0x9B,
  BIC_GPV3_P3V3_STBY3_VOLTAGE = 0x9C,
  BIC_GPV3_VR_P1V8_VOLTAGE = 0x9D,
  BIC_GPV3_VR_P0V84_VOLTAGE = 0x9E,
  BIC_GPV3_E1S_1_12V_VOLTAGE = 0xA1,
  BIC_GPV3_E1S_2_12V_VOLTAGE = 0xA5,
  BIC_GPV3_INA233_VOL_DEV0 = 0x81,
  BIC_GPV3_INA233_VOL_DEV1 = 0x85,
  BIC_GPV3_INA233_VOL_DEV2 = 0x89,
  BIC_GPV3_INA233_VOL_DEV3 = 0x8D,
  BIC_GPV3_INA233_VOL_DEV4 = 0xB1,
  BIC_GPV3_INA233_VOL_DEV5 = 0xB5,
  BIC_GPV3_INA233_VOL_DEV6 = 0xB9,
  BIC_GPV3_INA233_VOL_DEV7 = 0xBD,
  BIC_GPV3_INA233_VOL_DEV8 = 0xC1,
  BIC_GPV3_INA233_VOL_DEV9 = 0xC5,
  BIC_GPV3_INA233_VOL_DEV10 = 0xC9,
  BIC_GPV3_INA233_VOL_DEV11 = 0xCD,
  BIC_GPV3_ADC_P12V_STBY_VOL = 0x98,
  BIC_GPV3_ADC_P3V3_STBY_AUX_VOL = 0x99,
  BIC_GPV3_ADC_P1V8_VOL = 0x9F,
  BIC_GPV3_P3V3_STBY1_CURRENT = 0xAF,
  BIC_GPV3_P3V3_STBY2_CURRENT = 0xA8,
  BIC_GPV3_P3V3_STBY3_CURRENT = 0xA9,
  BIC_GPV3_VR_P1V8_CURRENT = 0xAC,
  BIC_GPV3_VR_P0V84_CURRENT = 0xAD,
  BIC_GPV3_E1S_1_12V_CURRENT = 0xA3,
  BIC_GPV3_E1S_2_12V_CURRENT = 0xA7,
  BIC_GPV3_P3V3_STBY1_POWER = 0x49,
  BIC_GPV3_P3V3_STBY2_POWER = 0x4A,
  BIC_GPV3_P3V3_STBY3_POWER = 0x4B,
  BIC_GPV3_VR_P1V8_POWER = 0x4C,
  BIC_GPV3_VR_P0V84_POWER = 0x4D,
  BIC_GPV3_E1S_1_12V_POWER = 0xA0,
  BIC_GPV3_E1S_2_12V_POWER = 0xA4,
  BIC_GPV3_INA233_PWR_DEV0 = 0x80,
  BIC_GPV3_INA233_PWR_DEV1 = 0x84,
  BIC_GPV3_INA233_PWR_DEV2 = 0x88,
  BIC_GPV3_INA233_PWR_DEV3 = 0x8C,
  BIC_GPV3_INA233_PWR_DEV4 = 0xB0,
  BIC_GPV3_INA233_PWR_DEV5 = 0xB4,
  BIC_GPV3_INA233_PWR_DEV6 = 0xB8,
  BIC_GPV3_INA233_PWR_DEV7 = 0xBC,
  BIC_GPV3_INA233_PWR_DEV8 = 0xC0,
  BIC_GPV3_INA233_PWR_DEV9 = 0xC4,
  BIC_GPV3_INA233_PWR_DEV10 = 0xC8,
  BIC_GPV3_INA233_PWR_DEV11 = 0xCC,

  //BIC Sierra Point Expansion Board Sensors
  BIC_SPE_SENSOR_SSD0_TEMP = 0x80,
  BIC_SPE_SENSOR_SSD1_TEMP = 0x81,
  BIC_SPE_SENSOR_SSD2_TEMP = 0x82,
  BIC_SPE_SENSOR_SSD3_TEMP = 0x83,
  BIC_SPE_SENSOR_SSD4_TEMP = 0x84,
  BIC_SPE_SENSOR_SSD5_TEMP = 0x85,
  BIC_SPE_SENSOR_INLET_TEMP = 0x91,
  BIC_SPE_SENSOR_12V_EDGE_VOL = 0x92,
  BIC_SPE_SENSOR_12V_MAIN_VOL = 0x93,
  BIC_SPE_SENSOR_3V3_EDGE_VOL = 0x94,
  BIC_SPE_SENSOR_3V3_MAIN_VOL = 0x95,
  BIC_SPE_SENSOR_3V3_STBY_VOL = 0x96,
  BIC_SPE_SENSOR_SSD0_CUR = 0x97,
  BIC_SPE_SENSOR_SSD1_CUR = 0x98,
  BIC_SPE_SENSOR_SSD2_CUR = 0x99,
  BIC_SPE_SENSOR_SSD3_CUR = 0x9A,
  BIC_SPE_SENSOR_SSD4_CUR = 0x9B,
  BIC_SPE_SENSOR_SSD5_CUR = 0x9C,
  BIC_SPE_SENSOR_12V_MAIN_CUR = 0x9D,

  //BIC BaseBoard Sensors
  BIC_BB_SENSOR_INLET_TEMP = 0xD1,
  BIC_BB_SENSOR_OUTLET_TEMP = 0xD2,
  BIC_BB_SENSOR_HSC_TEMP = 0xD3,
  BIC_BB_SENSOR_MEDUSA_VOUT = 0xD7,
  BIC_BB_SENSOR_HSC_VIN = 0xDC,
  BIC_BB_SENSOR_HSC_PIN = 0xDD,
  BIC_BB_SENSOR_HSC_IOUT = 0xDE,
  BIC_BB_SENSOR_P12V_MEDUSA_CUR = 0xDF,
  BIC_BB_SENSOR_P5V = 0xD4,
  BIC_BB_SENSOR_P12V = 0xD5,
  BIC_BB_SENSOR_P3V3_STBY = 0xD6,
  BIC_BB_SENSOR_MEDUSA_PIN = 0xD8,
  BIC_BB_SENSOR_P1V2_BMC_STBY = 0xD9,
  BIC_BB_SENSOR_P2V5_BMC_STBY = 0xDA,
  BIC_BB_SENSOR_MEDUSA_VIN = 0xDB,
  BIC_BB_SENSOR_MEDUSA_IOUT = 0xE0,

  //BMC - sensors
  BMC_SENSOR_INLET_TEMP = 0xED,
  BMC_SENSOR_OUTLET_TEMP = 0xEE,
  NIC_SENSOR_TEMP = 0xEF,
  BMC_SENSOR_FAN0_TACH = 0xE0,
  BMC_SENSOR_FAN1_TACH = 0xE1,
  BMC_SENSOR_FAN2_TACH = 0xE2,
  BMC_SENSOR_FAN3_TACH = 0xE3,
  BMC_SENSOR_FAN4_TACH = 0xE4,
  BMC_SENSOR_FAN5_TACH = 0xE5,
  BMC_SENSOR_FAN6_TACH = 0xE6,
  BMC_SENSOR_FAN7_TACH = 0xE7,
  BMC_SENSOR_PWM0      = 0xE8,
  BMC_SENSOR_PWM1      = 0xE9,
  BMC_SENSOR_PWM2      = 0xEA,
  BMC_SENSOR_PWM3      = 0xEB,
  BMC_SENSOR_HSC_PEAK_IOUT = 0xC8,
  BMC_SENSOR_HSC_PEAK_PIN  = 0xC9,
  BMC_SENSOR_FAN_PWR = 0xCA,
  BMC_SENSOR_HSC_EIN = 0xCB,
  BMC_SENSOR_PDB_DL_VDELTA = 0xCC,
  BMC_SENSOR_CURR_LEAKAGE = 0xCD,
  BMC_SENSOR_PDB_BB_VDELTA = 0xCE,
  BMC_SENSOR_MEDUSA_VDELTA = 0xCF,
  BMC_SENSOR_MEDUSA_CURR = 0xD0,
  BMC_SENSOR_MEDUSA_PWR = 0xD1,
  BMC_SENSOR_NIC_P12V = 0xD2,
  BMC_SENSOR_NIC_PWR = 0xD3,
  BMC_SENSOR_P1V0_STBY = 0xD4,
  BMC_SENSOR_P0V6_STBY = 0xD5,
  BMC_SENSOR_P3V3_RGM_STBY = 0xD6,
  BMC_SENSOR_P5V = 0xF0,
  BMC_SENSOR_P12V = 0xF1,
  BMC_SENSOR_P3V3_STBY = 0xF2,
  BMC_SENSOR_P1V8_STBY = 0xF3,
  BMC_SENSOR_P1V2_BMC_STBY = 0xF4,
  BMC_SENSOR_P2V5_BMC_STBY = 0xF5,
  BMC_SENSOR_MEDUSA_VOUT = 0xF6,
  BMC_SENSOR_HSC_VIN = 0xF7,
  BMC_SENSOR_HSC_TEMP = 0xF8,
  BMC_SENSOR_HSC_PIN = 0xF9,
  BMC_SENSOR_HSC_IOUT = 0xFA,
  BMC_SENSOR_FAN_IOUT = 0xFB,
  BMC_SENSOR_NIC_IOUT = 0xFC,
  BMC_SENSOR_MEDUSA_VIN = 0xFD,
};

//Serverboard Discrete/SEL Sensors
enum {
  BIC_SENSOR_SYSTEM_STATUS = 0x46,
  ME_SENSOR_SMART_CLST = 0xB2,
  BIC_SENSOR_VRHOT = 0xB4,
  BIC_SENSOR_PROC_FAIL = 0x65,
  BIC_SENSOR_SSD_HOT_PLUG = 0xED,
  BB_BIC_SENSOR_POWER_DETECT = 0xE1,
  BB_BIC_SENSOR_BUTTON_DETECT = 0xE2
};

// Aggregate sensors
enum {
  AGGREGATE_SENSOR_SYSTEM_AIRFLOW = 0x0,
};

//ADC INFO
enum {
  ADC0 = 0,
  ADC1,
  ADC2,
  ADC3,
  ADC4,
  ADC5,
  ADC6,
  ADC7,
  ADC8,
  ADC9,
  ADC10,
  ADC11,
  ADC12,
  ADC13,
  ADC14,
};

//GENERIC I2C Sensors
enum {
  TEMP_INLET = 0,
  TEMP_OUTLET,
  TEMP_NIC,
  TEMP_NICEXP_OUTLET,
};

//ADM1278 INFO
enum {
  ADM1278_VOLTAGE = 0,
  ADM1278_CURRENT,
  ADM1278_POWER,
  ADM1278_TEMP,
};

//ATTR INFO
enum {
  HSC_VOLTAGE = 0,
  HSC_CURRENT,
  HSC_POWER,
  HSC_TEMP,
};

typedef struct {
  uint8_t type;
  float m;
  float b;
  float r;
} PAL_ATTR_INFO;

//HSC INFO
enum {
  HSC_ID0 = 0,
  HSC_ID1,
};

typedef struct {
  uint8_t id;
  uint8_t slv_addr;
  PAL_ATTR_INFO* info;
} PAL_HSC_INFO;

typedef struct {
  uint8_t id;
  uint8_t bus;
  uint8_t slv_addr;
} PAL_I2C_BUS_INFO;

typedef struct {
  int integer :10;
  uint8_t fract :6;
} PAL_S10_6_FORMAT;

#endif
