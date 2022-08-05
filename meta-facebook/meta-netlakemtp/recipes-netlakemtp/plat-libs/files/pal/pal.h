/*
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
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

#ifndef __PAL_H__
#define __PAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <openbmc/obmc-pal.h>
#include <facebook/netlakemtp_common.h>
#include <facebook/netlakemtp_fruid.h>
#include "pal_power.h"

#define MAX_NUM_FRUS    (FRU_CNT-1)
#define MAX_NODES       1
#define FRUID_SIZE      512
#define CUSTOM_FRU_LIST 1
#define FRU_DEVICE_LIST 1
#define FAN_CTL_BUS     21
#define FAN_CTL_ADDR    0x42
#define FAN_CTL_ADDR_STR "21"
// Follow max31790 spec to set fan tach monitor
#define FAN_CONF_DATA   0x19
#define FAN_CHANNEL     6
#define MAX31790_REG_FAN_CONFIG(ch)  (0x02 + (ch))

#define MAX_FRU_CMD_STR   16

#define CPLD_VER_BYTE   4

// For FRUs missing error code
#define ERR_CODE_SERVER_MISSING          0xE2

#define MAX_NUM_ERR_CODES                256
#define MAX_NUM_ERR_CODES_ARRAY          32
#define ERR_CODE_BIN                     "/tmp/error_code.bin"
#define ERR_CODE_ENABLE                  1
#define ERR_CODE_DISABLE                 0

#define PATH_POWER_ON_RESET "/tmp/ast_power_on_reset"

// Define SENSOR_NA to distinguish whether sensor reading fail or not
#define SENSOR_NA    (-4)

#define PAGE_SIZE  0x1000

// System Control Unit (ADC Register
#define ADC_BASE         0x1E6E9000
#define REG_ADC0C        0x0c

// LPC control
#define AST_LPC_BASE 0x1e789000
#define HICRA_OFFSET 0x9C
#define HICRA_MASK_UART1 0x70000
#define HICRA_MASK_UART3 0x1C00000
#define HICRA_MASK_UART4 0xE000000

#define UART1_TO_UART3 0x5
#define UART3_TO_UART1 0x5

#define UART3_TO_UART4 0x6
#define UART4_TO_UART3 0x4

extern const char pal_fru_list_sensor_history[];
extern const char pal_fru_list[];
extern size_t pal_pwm_cnt;
extern size_t pal_tach_cnt;

enum {
  FRU_ABSENT           = 0,
  FRU_PRESENT,
};

enum {
  DEBUG_CARD_PRESENT = 0,
  DEBUG_CARD_ABSENT,
};

/*
TODO: Temporary. Error code needs to modify later on
*/
enum {
  ERR_CODE_FRU_SERVER_HEALTH = 0x05,
  ERR_CODE_FRU_PDB_HEALTH = 0x09,
};

enum {
  EVT = 0,
  EVT2 = 1,
  DVT = 2,
  PVT = 3,
  MP = 4,
  POC = 7,
};

int pal_check_gpio_prsnt(uint8_t gpio, int presnt_expect);
int pal_copy_eeprom_to_bin(const char *eeprom_file, const char *bin_file);
int pal_check_fru_is_valid(const char* fruid_path);
int pal_get_cpld_ver(uint8_t fru, char *rbuf);
int pal_post_display(uint8_t status);

int pal_set_id_led(uint8_t fru, enum LED_HIGH_ACTIVE value);
int pal_set_fault_led(uint8_t fru, enum LED_HIGH_ACTIVE value);
int pal_get_error_code(uint8_t *data, uint8_t* error_count);
void pal_set_error_code(unsigned char error_num, uint8_t error_code_status);

int8_t pal_set_uart_routing(uint8_t routing);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __PAL_H__ */
