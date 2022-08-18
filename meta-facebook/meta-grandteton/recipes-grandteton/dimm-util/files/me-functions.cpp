/*
 * Copyright 2022-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdint.h>
#include <openbmc/ipmi.h>
#include <openbmc/ipmb.h>
#include <openbmc/nm.h>
#include <openbmc/pal_def.h>
#include "dimm-util.h"
#include "dimm-util-plat.h"

static uint8_t spd_addr[MAX_DIMM_NUM_FBGT/2] = {
  DIMM0_SPD_ADDR,
  DIMM1_SPD_ADDR,
  DIMM2_SPD_ADDR,
  DIMM3_SPD_ADDR,
  DIMM4_SPD_ADDR,
  DIMM5_SPD_ADDR,
  DIMM6_SPD_ADDR,
  DIMM7_SPD_ADDR,
};

static uint8_t pmic_addr[MAX_DIMM_NUM_FBGT/2] = {
  DIMM0_PMIC_ADDR,
  DIMM1_PMIC_ADDR,
  DIMM2_PMIC_ADDR,
  DIMM3_PMIC_ADDR,
  DIMM4_PMIC_ADDR,
  DIMM5_PMIC_ADDR,
  DIMM6_PMIC_ADDR,
  DIMM7_PMIC_ADDR,
};

// dimm location constant strings, matching silk screen
static char const *dimm_label[NUM_CPU_FBGT][MAX_DIMM_NUM_FBGT] = {
  { "A0","C0", "A1","C1", "A2","C2", "A3","C3", "A4","C4", "A5","C5", "A6","C6", "A7","C7" },
  { "B0","D0", "B1","D1", "B2","D2", "B3","D3", "B4","D4", "B5","D5", "B6","D6", "B7","D7" },
};

static uint8_t dimm_cache_id[NUM_CPU_FBGT][MAX_DIMM_NUM_FBGT] = {
  {  0,1,   2,3,   4,5,   6,7,   8,9,  10,11, 12,13, 14,15 },
  { 16,17, 18,19, 20,21, 22,23, 24,25, 26,27, 28,29, 30,31 },
};

static char const *fru_name_fbgt[NUM_FRU_FBGT] = {
  "mb",
};

int
plat_init(void) {
  num_frus = NUM_FRU_FBGT;
  num_dimms_per_cpu = MAX_DIMM_NUM_FBGT;
  num_cpus          = NUM_CPU_FBGT;
  total_dimms       = num_dimms_per_cpu * num_cpus;
  fru_name   = fru_name_fbgt;
  fru_id_min = FRU_ID_MIN_FBGT;
  fru_id_max = FRU_ID_MAX_FBGT;
  fru_id_all = FRU_ID_ALL_FBGT;
  read_block = true;
  max_retries = 15;
  retry_intvl = 50;

  return 0;
}

const char *
get_dimm_label(uint8_t cpu, uint8_t dimm) {
  if ((cpu >= NUM_CPU_FBGT)  || (dimm >= MAX_DIMM_NUM_FBGT)) {
    return "N/A";
  }

  return dimm_label[cpu][dimm];
}

uint8_t
get_dimm_cache_id(uint8_t cpu, uint8_t dimm) {
  if ((cpu >= NUM_CPU_FBGT)  || (dimm >= MAX_DIMM_NUM_FBGT)) {
    return 0xff;
  }

  return dimm_cache_id[cpu][dimm];
}

int
util_read_spd(uint8_t fru_id, uint8_t cpu, uint8_t dimm, uint16_t offset, uint8_t len, uint8_t *rxbuf) {
  NM_RW_INFO info;
  NM_DIMM_SMB_DEV dev;

  info.bus = NM_IPMB_BUS_ID;
  info.nm_addr = NM_SLAVE_ADDR;
  info.bmc_addr = BMC_DEF_SLAVE_ADDR;

  dev.cpu = cpu;
  dev.bus_id = (dimm >= (MAX_DIMM_NUM_FBGT/2)) ? 1 : 0;
  dev.addr = spd_addr[dimm % (MAX_DIMM_NUM_FBGT/2)];
  dev.offset_len = 2;
  dev.offset = ((offset & 0x780) << 1) | (0x80 | (offset & 0x7F));
  if (cmd_NM_read_dimm_smbus(&info, &dev, len, rxbuf)) {
    return -1;
  }

  return len;
}

int
util_check_me_status(uint8_t fru_id) {
  NM_RW_INFO info;
  ipmi_dev_id_t dev_id;

  info.bus = NM_IPMB_BUS_ID;
  info.nm_addr = NM_SLAVE_ADDR;
  info.bmc_addr = BMC_DEF_SLAVE_ADDR;
  if (cmd_NM_get_dev_id(&info, &dev_id)) {
    return -1;
  }

  if ((dev_id.fw_rev1 & 0x80)) {  // bit7 = 0b: normal operation
    return -1;
  }

  return 0;
}

bool
is_pmic_supported(void) {
  return true;
}

int
util_read_pmic(uint8_t slot_id, uint8_t cpu, uint8_t dimm, uint8_t offset, uint8_t len, uint8_t *rxbuf) {
  NM_RW_INFO info;
  NM_DIMM_SMB_DEV dev;

  info.bus = NM_IPMB_BUS_ID;
  info.nm_addr = NM_SLAVE_ADDR;
  info.bmc_addr = BMC_DEF_SLAVE_ADDR;

  dev.cpu = cpu;
  dev.bus_id = (dimm >= (MAX_DIMM_NUM_FBGT/2)) ? 1 : 0;
  dev.addr = pmic_addr[dimm % (MAX_DIMM_NUM_FBGT/2)];
  dev.offset_len = 1;
  dev.offset = offset;
  if (cmd_NM_read_dimm_smbus(&info, &dev, len, rxbuf)) {
    return -1;
  }

  return len;
}

int
util_write_pmic(uint8_t slot_id, uint8_t cpu, uint8_t dimm, uint8_t offset, uint8_t len, uint8_t *txbuf) {
  NM_RW_INFO info;
  NM_DIMM_SMB_DEV dev;

  info.bus = NM_IPMB_BUS_ID;
  info.nm_addr = NM_SLAVE_ADDR;
  info.bmc_addr = BMC_DEF_SLAVE_ADDR;

  dev.cpu = cpu;
  dev.bus_id = (dimm >= (MAX_DIMM_NUM_FBGT/2)) ? 1 : 0;
  dev.addr = pmic_addr[dimm % (MAX_DIMM_NUM_FBGT/2)];
  dev.offset_len = 1;
  dev.offset = offset;
  if (cmd_NM_write_dimm_smbus(&info, &dev, len, txbuf)) {
    return -1;
  }

  return len;
}
