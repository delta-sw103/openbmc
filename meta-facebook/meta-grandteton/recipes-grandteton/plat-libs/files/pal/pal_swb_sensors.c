#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <libpldm-oem/pldm.h>
#include "pal_swb_sensors.h"
#include "pal.h"

static int
get_swb_sensor(uint8_t fru, uint8_t sensor_num, float *value) 
{
  uint8_t tbuf[255] = {0};
  uint8_t* rbuf = (uint8_t *) NULL;//, tmp;
  struct pldm_snr_reading_t* resp;
  uint8_t tlen = 0;
  size_t  rlen = 0;
  int16_t integer=0;
  float decimal=0;
int rc;

  struct pldm_msg* pldmbuf = (struct pldm_msg *)tbuf;
  pldmbuf->hdr.request = 1;
  pldmbuf->hdr.type    = PLDM_PLATFORM;
  pldmbuf->hdr.command = PLDM_GET_SENSOR_READING;
  tlen = PLDM_HEADER_SIZE;
  tbuf[tlen++] = sensor_num;
  tbuf[tlen++] = 0;
  tbuf[tlen++] = 0;

  rc = oem_pldm_send_recv(SWB_BUS_ID, SWB_BIC_EID, tbuf, tlen, &rbuf, &rlen);

  if (rc == 0) {
    resp= (struct pldm_snr_reading_t*)rbuf;
    if (resp->data.completion_code || resp->data.sensor_operational_state) {
      rc = -1;
      goto exit;
    }
    integer = resp->data.present_reading[0] | resp->data.present_reading[1] << 8;
    decimal = (float)(resp->data.present_reading[2] | resp->data.present_reading[3] << 8)/1000;

    if (integer > 0)
      *value = (float)integer + decimal;
    else
      *value = (float)integer - decimal;
  }

exit:
  if (rbuf)
    free(rbuf);

  return rc;
}

PAL_SENSOR_MAP swb_sensor_map[] = {
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"NIC0_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x01
  {"NIC0_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x02
  {"NIC0_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x03
  {"NIC0_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x04
  {"NIC1_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x05
  {"NIC1_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x06
  {"NIC1_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x07
  {"NIC1_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x08
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"NIC2_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x0A
  {"NIC2_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x0B
  {"NIC2_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x0C
  {"NIC2_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x0D
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"NIC3_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x11
  {"NIC3_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x12
  {"NIC3_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x13
  {"NIC3_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x14
  {"NIC4_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x15
  {"NIC4_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x16
  {"NIC4_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x17
  {"NIC4_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x18
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"NIC5_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x1A
  {"NIC5_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x1B
  {"NIC5_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x1C
  {"NIC5_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x1D
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"NIC6_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x21
  {"NIC6_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x22
  {"NIC6_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x23
  {"NIC6_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x24
  {"NIC7_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x25
  {"NIC7_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x26
  {"NIC7_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x27
  {"NIC7_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x28
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"HSC_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x2A
  {"HSC_VOUT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x2B
  {"HSC_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x2C
  {"HSC_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x2D
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"P12V_AUX", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x30
  {"P5V_AUX", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x31
  {"P3V3_AUX", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x32
  {"P1V2_AUX", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x33
  {"P3V3", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x34
  {"P1V8_PEX0", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x35
  {"P1V8_PEX1", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x36
  {"P1V8_PEX2", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x37
  {"P1V8_PEX3", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x38
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"PEX0_VR_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x40
  {"PEX0_P0V8_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x41
  {"PEX0_P0V8_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x42
  {"PEX0_P0V8_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x43
  {"PEX0_P1V25_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x44
  {"PEX0_P1V25_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x45
  {"PEX0_P1V25_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x46
  {"PEX1_VR_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x47
  {"PEX1_P0V8_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x48
  {"PEX1_P0V8_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x49
  {"PEX1_P0V8_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x4A
  {"PEX1_P1V25_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x4B
  {"PEX1_P1V25_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x4C
  {"PEX1_P1V25_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x4D
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"PEX2_VR_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x50
  {"PEX2_P0V8_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x51
  {"PEX2_P0V8_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x52
  {"PEX2_P0V8_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x53
  {"PEX2_P1V25_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x54
  {"PEX2_P1V25_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x55
  {"PEX2_P1V25_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x56
  {"PEX3_VR_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x57
  {"PEX3_P0V8_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x58
  {"PEX3_P0V8_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x59
  {"PEX3_P0V8_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x5A
  {"PEX3_P1V25_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x5B
  {"PEX3_P1V25_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x5C
  {"PEX3_P1V25_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x5D
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"PEX_P1V8_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x60
  {"PEX_P1V8_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x61
  {"PEX_P1V8_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x62
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"PEX0_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x6A
  {"PEX1_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x6B
  {"PEX2_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x6C
  {"PEX3_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x6D
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"SYSTEM_INLET_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x70
  {"OUTLET_TEMP_L1", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x71
  {"OUTLET_TEMP_L2", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x72
  {"OUTLET_TEMP_R1", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x73
  {"OUTLET_TEMP_R2", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x74
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {NULL, 0, NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0},
  {"E1S_0_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x80
  {"E1S_0_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x81
  {"E1S_0_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x82
  {"E1S_0_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x83
  {"E1S_1_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x84
  {"E1S_1_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x85
  {"E1S_1_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x86
  {"E1S_1_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x87
  {"E1S_2_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x88
  {"E1S_2_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x89
  {"E1S_2_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x8A
  {"E1S_2_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x8B
  {"E1S_3_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x8C
  {"E1S_3_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x8D
  {"E1S_3_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x8E
  {"E1S_3_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x8F
  {"E1S_4_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x90
  {"E1S_4_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x91
  {"E1S_4_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x92
  {"E1S_4_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x93
  {"E1S_5_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x94
  {"E1S_5_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x95
  {"E1S_5_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x96
  {"E1S_5_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x97
  {"E1S_6_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x98
  {"E1S_6_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x99
  {"E1S_6_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x9A
  {"E1S_6_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x9B
  {"E1S_7_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0x9C
  {"E1S_7_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0x9D
  {"E1S_7_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0x9E
  {"E1S_7_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0x9F
  {"E1S_8_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0xA0
  {"E1S_8_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0xA1
  {"E1S_8_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0xA2
  {"E1S_8_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0xA3
  {"E1S_9_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0xA4
  {"E1S_9_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0xA5
  {"E1S_9_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0xA6
  {"E1S_9_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0xA7
  {"E1S_10_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0xA8
  {"E1S_10_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0xA9
  {"E1S_10_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0xAA
  {"E1S_10_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0xAB
  {"E1S_11_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0xAC
  {"E1S_11_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0xAD
  {"E1S_11_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0xAE
  {"E1S_11_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0xAF
  {"E1S_12_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0xB0
  {"E1S_12_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0xB1
  {"E1S_12_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0xB2
  {"E1S_12_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0xB3
  {"E1S_13_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0xB4
  {"E1S_13_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0xB5
  {"E1S_13_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0xB6
  {"E1S_13_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0xB7
  {"E1S_14_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0xB8
  {"E1S_14_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0xB9
  {"E1S_14_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0xBA
  {"E1S_14_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0xBB
  {"E1S_15_TEMP", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, TEMP}, // 0xBC
  {"E1S_15_VOLT", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, VOLT}, // 0xBD
  {"E1S_15_CURR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, CURR}, // 0xBE
  {"E1S_15_PWR", 0, get_swb_sensor, 0, {0, 0, 0, 0, 0, 0, 0, 0}, POWER}, // 0xBF
};

const uint8_t swb_sensor_list[] = {
  SWB_SENSOR_TEMP_NIC_0,
  SWB_SENSOR_VOLT_NIC_0,
  SWB_SENSOR_IOUT_NIC_0,
  SWB_SENSOR_POUT_NIC_0,
  SWB_SENSOR_TEMP_NIC_1,
  SWB_SENSOR_VOLT_NIC_1,
  SWB_SENSOR_IOUT_NIC_1,
  SWB_SENSOR_POUT_NIC_1,
  SWB_SENSOR_TEMP_NIC_2,
  SWB_SENSOR_VOLT_NIC_2,
  SWB_SENSOR_IOUT_NIC_2,
  SWB_SENSOR_POUT_NIC_2,
  SWB_SENSOR_TEMP_NIC_3,
  SWB_SENSOR_VOLT_NIC_3,
  SWB_SENSOR_IOUT_NIC_3,
  SWB_SENSOR_POUT_NIC_3,
  SWB_SENSOR_TEMP_NIC_4,
  SWB_SENSOR_VOLT_NIC_4,
  SWB_SENSOR_IOUT_NIC_4,
  SWB_SENSOR_POUT_NIC_4,
  SWB_SENSOR_TEMP_NIC_5,
  SWB_SENSOR_VOLT_NIC_5,
  SWB_SENSOR_IOUT_NIC_5,
  SWB_SENSOR_POUT_NIC_5,
  SWB_SENSOR_TEMP_NIC_6,
  SWB_SENSOR_VOLT_NIC_6,
  SWB_SENSOR_IOUT_NIC_6,
  SWB_SENSOR_POUT_NIC_6,
  SWB_SENSOR_TEMP_NIC_7,
  SWB_SENSOR_VOLT_NIC_7,
  SWB_SENSOR_IOUT_NIC_7,
  SWB_SENSOR_POUT_NIC_7,
  SWB_SENSOR_TEMP_PDB_HSC,
  SWB_SENSOR_VOUT_PDB_HSC,
  SWB_SENSOR_IOUT_PDB_HSC,
  SWB_SENSOR_POUT_PDB_HSC,
  SWB_SENSOR_BB_P12V_AUX,
  SWB_SENSOR_BB_P5V_AUX,
  SWB_SENSOR_BB_P3V3_AUX,
  SWB_SENSOR_BB_P1V2_AUX,
  SWB_SENSOR_BB_P3V3,
  SWB_SENSOR_BB_P1V8_PEX0,
  SWB_SENSOR_BB_P1V8_PEX1,
  SWB_SENSOR_BB_P1V8_PEX2,
  SWB_SENSOR_BB_P1V8_PEX3,
  SWB_SENSOR_VR_TEMP_PEX_0,
  SWB_SENSOR_P0V8_VOLT_PEX_0,
  SWB_SENSOR_P0V8_IOUT_PEX_0,
  SWB_SENSOR_P0V8_POUT_PEX_0,
  SWB_SENSOR_P1V25_VOLT_PEX_0,
  SWB_SENSOR_P1V25_IOUT_PEX_0,
  SWB_SENSOR_P1V25_POUT_PEX_0,
  SWB_SENSOR_VR_TEMP_PEX_1,
  SWB_SENSOR_P0V8_VOLT_PEX_1,
  SWB_SENSOR_P0V8_IOUT_PEX_1,
  SWB_SENSOR_P0V8_POUT_PEX_1,
  SWB_SENSOR_P1V25_VOLT_PEX_1,
  SWB_SENSOR_P1V25_IOUT_PEX_1,
  SWB_SENSOR_P1V25_POUT_PEX_1,
  SWB_SENSOR_VR_TEMP_PEX_2,
  SWB_SENSOR_P0V8_VOLT_PEX_2,
  SWB_SENSOR_P0V8_IOUT_PEX_2,
  SWB_SENSOR_P0V8_POUT_PEX_2,
  SWB_SENSOR_P1V25_VOLT_PEX_2,
  SWB_SENSOR_P1V25_IOUT_PEX_2,
  SWB_SENSOR_P1V25_POUT_PEX_2,
  SWB_SENSOR_VR_TEMP_PEX_3,
  SWB_SENSOR_P0V8_VOLT_PEX_3,
  SWB_SENSOR_P0V8_IOUT_PEX_3,
  SWB_SENSOR_P0V8_POUT_PEX_3,
  SWB_SENSOR_P1V25_VOLT_PEX_3,
  SWB_SENSOR_P1V25_IOUT_PEX_3,
  SWB_SENSOR_P1V25_POUT_PEX_3,
  SWB_SENSOR_P1V8_VOLT_PEX,
  SWB_SENSOR_P1V8_IOUT_PEX,
  SWB_SENSOR_P1V8_POUT_PEX,
  SWB_SENSOR_TEMP_PEX_0,
  SWB_SENSOR_TEMP_PEX_1,
  SWB_SENSOR_TEMP_PEX_2,
  SWB_SENSOR_TEMP_PEX_3,
  SWB_SENSOR_SYSTEM_INLET_TEMP,
  SWB_SENSOR_OUTLET_TEMP_L1,
  SWB_SENSOR_OUTLET_TEMP_L2,
  SWB_SENSOR_OUTLET_TEMP_R1,
  SWB_SENSOR_OUTLET_TEMP_R2,
  SWB_SENSOR_TEMP_E1S_0,
  SWB_SENSOR_VOLT_E1S_0,
  SWB_SENSOR_CURR_E1S_0,
  SWB_SENSOR_POUT_E1S_0,
  SWB_SENSOR_TEMP_E1S_1,
  SWB_SENSOR_VOLT_E1S_1,
  SWB_SENSOR_CURR_E1S_1,
  SWB_SENSOR_POUT_E1S_1,
  SWB_SENSOR_TEMP_E1S_2,
  SWB_SENSOR_VOLT_E1S_2,
  SWB_SENSOR_CURR_E1S_2,
  SWB_SENSOR_POUT_E1S_2,
  SWB_SENSOR_TEMP_E1S_3,
  SWB_SENSOR_VOLT_E1S_3,
  SWB_SENSOR_CURR_E1S_3,
  SWB_SENSOR_POUT_E1S_3,
  SWB_SENSOR_TEMP_E1S_4,
  SWB_SENSOR_VOLT_E1S_4,
  SWB_SENSOR_CURR_E1S_4,
  SWB_SENSOR_POUT_E1S_4,
  SWB_SENSOR_TEMP_E1S_5,
  SWB_SENSOR_VOLT_E1S_5,
  SWB_SENSOR_CURR_E1S_5,
  SWB_SENSOR_POUT_E1S_5,
  SWB_SENSOR_TEMP_E1S_6,
  SWB_SENSOR_VOLT_E1S_6,
  SWB_SENSOR_CURR_E1S_6,
  SWB_SENSOR_POUT_E1S_6,
  SWB_SENSOR_TEMP_E1S_7,
  SWB_SENSOR_VOLT_E1S_7,
  SWB_SENSOR_CURR_E1S_7,
  SWB_SENSOR_POUT_E1S_7,
  SWB_SENSOR_TEMP_E1S_8,
  SWB_SENSOR_VOLT_E1S_8,
  SWB_SENSOR_CURR_E1S_8,
  SWB_SENSOR_POUT_E1S_8,
  SWB_SENSOR_TEMP_E1S_9,
  SWB_SENSOR_VOLT_E1S_9,
  SWB_SENSOR_CURR_E1S_9,
  SWB_SENSOR_POUT_E1S_9,
  SWB_SENSOR_TEMP_E1S_10,
  SWB_SENSOR_VOLT_E1S_10,
  SWB_SENSOR_CURR_E1S_10,
  SWB_SENSOR_POUT_E1S_10,
  SWB_SENSOR_TEMP_E1S_11,
  SWB_SENSOR_VOLT_E1S_11,
  SWB_SENSOR_CURR_E1S_11,
  SWB_SENSOR_POUT_E1S_11,
  SWB_SENSOR_TEMP_E1S_12,
  SWB_SENSOR_VOLT_E1S_12,
  SWB_SENSOR_CURR_E1S_12,
  SWB_SENSOR_POUT_E1S_12,
  SWB_SENSOR_TEMP_E1S_13,
  SWB_SENSOR_VOLT_E1S_13,
  SWB_SENSOR_CURR_E1S_13,
  SWB_SENSOR_POUT_E1S_13,
  SWB_SENSOR_TEMP_E1S_14,
  SWB_SENSOR_VOLT_E1S_14,
  SWB_SENSOR_CURR_E1S_14,
  SWB_SENSOR_POUT_E1S_14,
  SWB_SENSOR_TEMP_E1S_15,
  SWB_SENSOR_VOLT_E1S_15,
  SWB_SENSOR_CURR_E1S_15,
  SWB_SENSOR_POUT_E1S_15,
};

size_t swb_sensor_cnt = sizeof(swb_sensor_list)/sizeof(uint8_t);
