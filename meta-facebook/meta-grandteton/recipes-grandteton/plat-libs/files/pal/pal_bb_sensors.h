#ifndef __PAL_BB_SENSORS_H__
#define __PAL_BB_SENSORS_H__

//Sensor Table
enum {
//NIC SENSOR
  NIC0_SNR_MEZZ0_P3V3_VOUT = 0x80,
  NIC0_SNR_MEZZ0_P12V_VOUT = 0x81,
  NIC0_SNR_MEZZ0_P12V_IOUT = 0x82,
  NIC0_SNR_MEZZ0_P12V_POUT = 0x83,
  NIC0_SNR_MEZZ0_TEMP = 0x84,

  NIC1_SNR_MEZZ1_P3V3_VOUT = 0x88,
  NIC1_SNR_MEZZ1_P12V_VOUT = 0x89,
  NIC1_SNR_MEZZ1_P12V_IOUT = 0x8A,
  NIC1_SNR_MEZZ1_P12V_POUT = 0x8B,
  NIC1_SNR_MEZZ1_TEMP = 0x8C,

//PDBV SENSOR
  PDBV_SNR_HSC0_VIN = 0x90,
  PDBV_SNR_HSC0_IOUT = 0x91,
  PDBV_SNR_HSC0_PIN = 0x92,
  PDBV_SNR_HSC0_TEMP = 0x93,
  PDBV_SNR_HSC0_PEAK_PIN = 0x94,

  PDBV_SNR_BRICK0_VOUT = 0x95,
  PDBV_SNR_BRICK0_IOUT = 0x96,
  PDBV_SNR_BRICK0_TEMP = 0x97,
  PDBV_SNR_BRICK0_POUT = 0x98,
  PDBV_SNR_BRICK1_VOUT = 0x99,
  PDBV_SNR_BRICK1_IOUT = 0x9A,
  PDBV_SNR_BRICK1_TEMP = 0x9B,
  PDBV_SNR_BRICK1_POUT = 0x9C,
  PDBV_SNR_BRICK2_VOUT = 0x9D,
  PDBV_SNR_BRICK2_IOUT = 0x9E,
  PDBV_SNR_BRICK2_TEMP = 0x9F,
  PDBV_SNR_BRICK2_POUT = 0xA0,
  PDBV_SNR_ADC128_P3V3_AUX =0xA1,

//PDBH SENSOR
  PDBH_SNR_HSC1_VIN = 0xB0,
  PDBH_SNR_HSC1_IOUT = 0xB1,
  PDBH_SNR_HSC1_PIN = 0xB2,
  PDBH_SNR_HSC1_TEMP = 0xB3,
  PDBH_SNR_HSC1_PEAK_PIN = 0xB4,

  PDBH_SNR_HSC2_VIN = 0xB5,
  PDBH_SNR_HSC2_IOUT = 0xB6,
  PDBH_SNR_HSC2_PIN = 0xB7,
  PDBH_SNR_HSC2_TEMP = 0xB8,
  PDBH_SNR_HSC2_PEAK_PIN = 0xB9,

//FAN SENSOR
  FAN_SNR_START_INDEX = 0xD0,
  BP0_SNR_FAN0_INLET_SPEED = 0xD0,
  BP0_SNR_FAN0_OUTLET_SPEED = 0xD1,
  BP0_SNR_FAN1_INLET_SPEED = 0xD2,
  BP0_SNR_FAN1_OUTLET_SPEED = 0xD3,
  BP1_SNR_FAN2_INLET_SPEED = 0xD4,
  BP1_SNR_FAN2_OUTLET_SPEED = 0xD5,
  BP1_SNR_FAN3_INLET_SPEED = 0xD6,
  BP1_SNR_FAN3_OUTLET_SPEED = 0xD7,
  BP0_SNR_FAN4_INLET_SPEED = 0xD8,
  BP0_SNR_FAN4_OUTLET_SPEED = 0xD9,
  BP0_SNR_FAN5_INLET_SPEED = 0xDA,
  BP0_SNR_FAN5_OUTLET_SPEED = 0xDB,
  BP1_SNR_FAN6_INLET_SPEED = 0xDC,
  BP1_SNR_FAN6_OUTLET_SPEED = 0xDD,
  BP1_SNR_FAN7_INLET_SPEED = 0xDE,
  BP1_SNR_FAN7_OUTLET_SPEED = 0xDF,

  BP0_SNR_FAN8_INLET_SPEED = 0xE0,
  BP0_SNR_FAN8_OUTLET_SPEED = 0xE1,
  BP0_SNR_FAN9_INLET_SPEED = 0xE2,
  BP0_SNR_FAN9_OUTLET_SPEED = 0xE3,
  BP1_SNR_FAN10_INLET_SPEED = 0xE4,
  BP1_SNR_FAN10_OUTLET_SPEED = 0xE5,
  BP1_SNR_FAN11_INLET_SPEED = 0xE6,
  BP1_SNR_FAN11_OUTLET_SPEED = 0xE7,
  BP0_SNR_FAN12_INLET_SPEED = 0xE8,
  BP0_SNR_FAN12_OUTLET_SPEED = 0xE9,
  BP0_SNR_FAN13_INLET_SPEED = 0xEA,
  BP0_SNR_FAN13_OUTLET_SPEED = 0xEB,
  BP1_SNR_FAN14_INLET_SPEED = 0xEC,
  BP1_SNR_FAN14_OUTLET_SPEED = 0xED,
  BP1_SNR_FAN15_INLET_SPEED = 0xEE,
  BP1_SNR_FAN15_OUTLET_SPEED = 0xEF,

 //SCM ADC
  SCM_SNR_P12V = 0xF0,
  SCM_SNR_P5V  = 0xF1,
  SCM_SNR_P3V3 = 0xF2,
  SCM_SNR_P2V5 = 0xF3,
  SCM_SNR_P1V8 = 0xF4,
  SCM_SNR_PGPPA = 0xF5,
  SCM_SNR_P1V2 = 0xF6,
  SCM_SNR_P1V0 = 0xF7,
 //SCM Voltage
  SCM_SNR_BMC_P12V_VOUT = 0xF8,
  SCM_SNR_BMC_P12V_IOUT = 0xF9,
  SCM_SNR_BMC_P12V_POUT = 0xFA,
 //SCM TEMP
  SCM_SNR_BMC_TEMP = 0xFB
};


//GENERIC I2C Sensors
enum {
  TEMP_BMC,
  TEMP_MEZZ0,
  TEMP_MEZZ1,
};

//NIC INFO
enum {
  MEZZ0 = 0,
  MEZZ1,
  MEZZ_CNT,
};

//48V HSC INFO
enum {
  HSC_48V_ID0,
  HSC_48V_ID1,
  HSC_48V_ID2,
  HSC_48V_CNT,
};

//BRICK INFO
enum {
  BRICK_ID0,
  BRICK_ID1,
  BRICK_ID2,
  BRICK_CNT,
};

//FAN IC ID
enum {
  FAN_CHIP_ID0,
  FAN_CHIP_ID1,
  FAN_CHIP_ID2,
  FAN_CHIP_ID3,
};

//FAN INFO
enum {
  FAN_TACH_ID0 = 0,
  FAN_TACH_ID1,
  FAN_TACH_ID2,
  FAN_TACH_ID3,
  FAN_TACH_ID4,
  FAN_TACH_ID5,
  FAN_TACH_ID6,
  FAN_TACH_ID7,
  FAN_TACH_ID8,
  FAN_TACH_ID9,
  FAN_TACH_ID10,
  FAN_TACH_ID11,
  FAN_TACH_ID12,
  FAN_TACH_ID13,
  FAN_TACH_ID14,
  FAN_TACH_ID15,
  FAN_TACH_ID16,
  FAN_TACH_ID17,
  FAN_TACH_ID18,
  FAN_TACH_ID19,
  FAN_TACH_ID20,
  FAN_TACH_ID21,
  FAN_TACH_ID22,
  FAN_TACH_ID23,
  FAN_TACH_ID24,
  FAN_TACH_ID25,
  FAN_TACH_ID26,
  FAN_TACH_ID27,
  FAN_TACH_ID28,
  FAN_TACH_ID29,
  FAN_TACH_ID30,
  FAN_TACH_ID31,
  FAN_TACH_CNT,
};
enum {
  FAN_PWM_ID0 = 0,
  FAN_PWM_ID1,
  FAN_PWM_ID2,
  FAN_PWM_ID3,
  FAN_PWM_ID4,
  FAN_PWM_ID5,
  FAN_PWM_ID6,
  FAN_PWM_ID7,
  FAN_PWM_ID8,
  FAN_PWM_ID9,
  FAN_PWM_ID10,
  FAN_PWM_ID11,
  FAN_PWM_ID12,
  FAN_PWM_ID13,
  FAN_PWM_ID14,
  FAN_PWM_ID15,
  FAN_PWM_CNT,
};

int pal_sensor_monitor_initial(void);
bool pal_check_nic_prsnt(uint8_t fru);

extern size_t pal_pwm_cnt;
extern size_t pal_tach_cnt;
extern const char pal_pwm_list[];
extern const char pal_tach_list[];
#endif
