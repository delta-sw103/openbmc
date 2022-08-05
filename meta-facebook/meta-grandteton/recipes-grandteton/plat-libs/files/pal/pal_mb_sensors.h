#ifndef __PAL_MB_SENSORS_H__
#define __PAL_MB_SENSORS_H__

enum {
  DIMM_ID0 = 0,
  DIMM_ID1,
  DIMM_ID2,
  DIMM_ID3,
  DIMM_ID4,
  DIMM_ID5,
  DIMM_ID6,
  DIMM_ID7,
  DIMM_ID8,
  DIMM_ID9,
  DIMM_ID10,
  DIMM_ID11,
  DIMM_ID12,
  DIMM_ID13,
  DIMM_ID14,
  DIMM_ID15,
  DIMM_ID_MAX,
};

enum {
  CFG_SAMPLE_DIABLE = 0,
  CFG_SAMPLE_2,
  CFG_SAMPLE_4,
  CFG_SAMPLE_8,
  CFG_SAMPLE_16,
  CFG_SAMPLE_32,
  CFG_SAMPLE_64,
  CFG_SAMPLE_128,
};

//MP5990 CMD INFO
#define MP5990_SLAVE_ADDR               (0x40)
#define MP5990_RSENSE                   (0.15)

#define HSC_PEAK_PIN                    (0xDA)
#define PMON_CFG_VIN_EN	                (1 << 2)
#define PMON_CFG_TEPM1_EN               (1 << 3)
#define PMON_CFG_CONTINUOUS_SAMPLE      (1 << 4)
#define PMON_CFG_VI_AVG(x)              (x << 8)
#define PMON_CFG_AVG_PWR(x)             (x << 11)
#define IOUT_OC_WARN_EN1                (1 << 10)


//FAN
#define FAN_CHIP_CNT                    (4)

//Sensor Table
enum {
//MB_SENSOR
  MB_SNR_E1S_P3V3_VOUT = 0x01,
  MB_SNR_E1S_P12V_VOUT = 0x02,
  MB_SNR_E1S_P12V_IOUT = 0x03,
  MB_SNR_E1S_P12V_POUT = 0x04,
  MB_SNR_E1S_TEMP = 0x05,

  MB_SNR_POWER_FAIL = 0x09,
  MB_SNR_PROCESSOR_FAIL = 0x0A,

//MB HSC
  MB_SNR_HSC_VIN = 0x0B,
  MB_SNR_HSC_IOUT = 0x0C,
  MB_SNR_HSC_PIN = 0x0D,
  MB_SNR_HSC_TEMP = 0x0E,
  MB_SNR_HSC_PEAK_PIN = 0x0F,

//MB TEMP
  MB_SNR_INLET_TEMP_R = 0x10,
  MB_SNR_INLET_TEMP_L = 0x11,
  MB_SNR_OUTLET_TEMP_R = 0x12,
  MB_SNR_OUTLET_TEMP_L = 0x13,

//MB CPU TEMP
  MB_SNR_CPU0_TEMP = 0x18,
  MB_SNR_CPU1_TEMP = 0x19,
  MB_SNR_CPU0_THERM_MARGIN = 0x1A,
  MB_SNR_CPU1_THERM_MARGIN = 0x1B,

//MB CPU POWER
  MB_SNR_CPU0_TJMAX = 0x1C,
  MB_SNR_CPU1_TJMAX = 0x1D,
  MB_SNR_CPU0_PKG_POWER = 0x1E,
  MB_SNR_CPU1_PKG_POWER = 0x1F,

//MB DIMM TEMP
  DIMM_SNR_START_INDEX = 0x20,
  MB_SNR_DIMM_CPU0_GRPA_TEMP = 0x20,
  MB_SNR_DIMM_CPU0_GRPB_TEMP = 0x21,
  MB_SNR_DIMM_CPU0_GRPC_TEMP = 0x22,
  MB_SNR_DIMM_CPU0_GRPD_TEMP = 0x23,
  MB_SNR_DIMM_CPU0_GRPE_TEMP = 0x24,
  MB_SNR_DIMM_CPU0_GRPF_TEMP = 0x25,
  MB_SNR_DIMM_CPU0_GRPG_TEMP = 0x26,
  MB_SNR_DIMM_CPU0_GRPH_TEMP = 0x27,
  MB_SNR_DIMM_CPU1_GRPA_TEMP = 0x28,
  MB_SNR_DIMM_CPU1_GRPB_TEMP = 0x29,
  MB_SNR_DIMM_CPU1_GRPC_TEMP = 0x2A,
  MB_SNR_DIMM_CPU1_GRPD_TEMP = 0x2B,
  MB_SNR_DIMM_CPU1_GRPE_TEMP = 0x2C,
  MB_SNR_DIMM_CPU1_GRPF_TEMP = 0x2D,
  MB_SNR_DIMM_CPU1_GRPG_TEMP = 0x2E,
  MB_SNR_DIMM_CPU1_GRPH_TEMP = 0x2F,

//MB CPU0 VR
  MB_SNR_VR_CPU0_VCCIN_VOLT = 0x30,
  MB_SNR_VR_CPU0_VCCIN_TEMP = 0x31,
  MB_SNR_VR_CPU0_VCCIN_CURR = 0x32,
  MB_SNR_VR_CPU0_VCCIN_POWER = 0x33,
  MB_SNR_VR_CPU0_VCCFA_FIVRA_VOLT = 0x34,
  MB_SNR_VR_CPU0_VCCFA_FIVRA_TEMP = 0x35,
  MB_SNR_VR_CPU0_VCCFA_FIVRA_CURR = 0x36,
  MB_SNR_VR_CPU0_VCCFA_FIVRA_POWER = 0x37,
  MB_SNR_VR_CPU0_VCCIN_FAON_VOLT = 0x38,
  MB_SNR_VR_CPU0_VCCIN_FAON_TEMP = 0x39,
  MB_SNR_VR_CPU0_VCCIN_FAON_CURR = 0x3A,
  MB_SNR_VR_CPU0_VCCIN_FAON_POWER = 0x3B,
  MB_SNR_VR_CPU0_VCCFA_VOLT = 0x3C,
  MB_SNR_VR_CPU0_VCCFA_TEMP = 0x3D,
  MB_SNR_VR_CPU0_VCCFA_CURR = 0x3E,
  MB_SNR_VR_CPU0_VCCFA_POWER = 0x3F,
  MB_SNR_VR_CPU0_VCCD_HV_VOLT = 0x40,
  MB_SNR_VR_CPU0_VCCD_HV_TEMP = 0x41,
  MB_SNR_VR_CPU0_VCCD_HV_CURR = 0x42,
  MB_SNR_VR_CPU0_VCCD_HV_POWER = 0x43,

//MB ADC128D818
  MB_SNR_ADC128_P12V_AUX = 0x44,
  NB_SNR_ADC128_P5V = 0x45,
  MB_SNR_ADC128_P3V3 = 0x46,
  MB_SNR_ADC128_P3V3_AUX = 0x47,

//MB CPU1 VR
  MB_SNR_VR_CPU1_VCCIN_VOLT = 0x48,
  MB_SNR_VR_CPU1_VCCIN_TEMP = 0x49,
  MB_SNR_VR_CPU1_VCCIN_CURR = 0x4A,
  MB_SNR_VR_CPU1_VCCIN_POWER = 0x4B,
  MB_SNR_VR_CPU1_VCCFA_FIVRA_VOLT = 0x4C,
  MB_SNR_VR_CPU1_VCCFA_FIVRA_TEMP = 0x4D,
  MB_SNR_VR_CPU1_VCCFA_FIVRA_CURR = 0x4E,
  MB_SNR_VR_CPU1_VCCFA_FIVRA_POWER = 0x4F,
  MB_SNR_VR_CPU1_VCCIN_FAON_VOLT = 0x50,
  MB_SNR_VR_CPU1_VCCIN_FAON_TEMP = 0x51,
  MB_SNR_VR_CPU1_VCCIN_FAON_CURR = 0x52,
  MB_SNR_VR_CPU1_VCCIN_FAON_POWER = 0x53,
  MB_SNR_VR_CPU1_VCCFA_VOLT = 0x54,
  MB_SNR_VR_CPU1_VCCFA_TEMP = 0x55,
  MB_SNR_VR_CPU1_VCCFA_CURR = 0x56,
  MB_SNR_VR_CPU1_VCCFA_POWER = 0x57,
  MB_SNR_VR_CPU1_VCCD_HV_VOLT = 0x58,
  MB_SNR_VR_CPU1_VCCD_HV_TEMP = 0x59,
  MB_SNR_VR_CPU1_VCCD_HV_CURR = 0X5A,
  MB_SNR_VR_CPU1_VCCD_HV_POWER = 0X5B,

//MB ADC BAT
  MB_SNR_P3V_BAT =0x5E,
//MB PCH TEMP
  MB_SNR_PCH_TEMP = 0x5F,

  MB_SNR_DIMM_CPU0_A0_POWER = 0x60,
  MB_SNR_DIMM_CPU0_C0_POWER = 0x61,
  MB_SNR_DIMM_CPU0_A1_POWER = 0x62,
  MB_SNR_DIMM_CPU0_C1_POWER = 0x63,
  MB_SNR_DIMM_CPU0_A2_POWER = 0x64,
  MB_SNR_DIMM_CPU0_C2_POWER = 0x65,
  MB_SNR_DIMM_CPU0_A3_POWER = 0x66,
  MB_SNR_DIMM_CPU0_C3_POWER = 0x67,
  MB_SNR_DIMM_CPU0_A4_POWER = 0x68,
  MB_SNR_DIMM_CPU0_C4_POWER = 0x69,
  MB_SNR_DIMM_CPU0_A5_POWER = 0x6A,
  MB_SNR_DIMM_CPU0_C5_POWER = 0x6B,
  MB_SNR_DIMM_CPU0_A6_POWER = 0x6C,
  MB_SNR_DIMM_CPU0_C6_POWER = 0x6D,
  MB_SNR_DIMM_CPU0_A7_POWER = 0x6E,
  MB_SNR_DIMM_CPU0_C7_POWER = 0x6F,

  MB_SNR_DIMM_CPU1_B0_POWER = 0x70,
  MB_SNR_DIMM_CPU1_D0_POWER = 0x71,
  MB_SNR_DIMM_CPU1_B1_POWER = 0x72,
  MB_SNR_DIMM_CPU1_D1_POWER = 0x73,
  MB_SNR_DIMM_CPU1_B2_POWER = 0x74,
  MB_SNR_DIMM_CPU1_D2_POWER = 0x75,
  MB_SNR_DIMM_CPU1_B3_POWER = 0x76,
  MB_SNR_DIMM_CPU1_D3_POWER = 0x77,
  MB_SNR_DIMM_CPU1_B4_POWER = 0x78,
  MB_SNR_DIMM_CPU1_D4_POWER = 0x79,
  MB_SNR_DIMM_CPU1_B5_POWER = 0x7A,
  MB_SNR_DIMM_CPU1_D5_POWER = 0x7B,
  MB_SNR_DIMM_CPU1_B6_POWER = 0x7C,
  MB_SNR_DIMM_CPU1_D6_POWER = 0x7D,
  MB_SNR_DIMM_CPU1_B7_POWER = 0x7E,
  MB_SNR_DIMM_CPU1_D7_POWER = 0x7F,
};


enum {
  ADC128_0 = 0,
  ADC128_NUM_CNT,
};


//GENERIC I2C Sensors
enum {
  TEMP_OUTLET_L = 0,
  TEMP_INLET_L,
  TEMP_OUTLET_R,
  TEMP_INLET_R,
};



//NM ID
enum {
  NM_ID0,
};

//PCH SENSOR INFO
enum {
  NM_PCH_TEMP = 8,
};

typedef struct {
  uint8_t id;
  uint8_t addr;
  uint8_t identify;
} PAL_DIMM_PMIC_INFO;


int read_adc128_val(uint8_t fru, uint8_t sensor_num, float *value);
int oper_adc128_power(uint8_t fru, uint8_t volt_snr_num, uint8_t curr_snr_num, float *value);
int read_dpm_vout(uint8_t fru, uint8_t sensor_num, float *value);
int read_adc_val(uint8_t fru, uint8_t sensor_num, float *value);
#endif
