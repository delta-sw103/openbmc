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

#include <openbmc/obmc-pal.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "pal_sensors.h"
#include "pal_health.h"
#include "pal_power.h"
#include "pal_def.h"
#include "pal_cfg.h"

#define CMD_GET_MAIN_CPLD_VER   (0x01)
#define MAIN_CPLD_SLV_ADDR      (0x80)
#define MAIN_CPLD_BUS_NUM       (7)

extern const char pal_fru_list[];

int pal_set_id_led(uint8_t fru, uint8_t status);
int pal_set_fault_led(uint8_t fru, uint8_t status);
int pal_get_rst_btn(uint8_t *status);
int pal_postcode_select(int option);
int pal_uart_select_led_set(void);
int pal_get_platform_id(uint8_t *id);
int pal_get_mb_position(uint8_t* pos);
int pal_get_board_rev_id(uint8_t *id);
void fru_eeprom_mb_check(char* mb_path);
bool is_cpu_socket_occupy(uint8_t cpu_idx);
int pal_get_syscfg_text(char *text);
int pal_peer_tray_get_lan_config(uint8_t sel, uint8_t *buf, uint8_t *rlen);
int pal_get_target_bmc_addr(uint8_t *tar_bmc_addr);
bool pal_skip_access_me(void);
int pal_get_pwr_btn(uint8_t *status);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __PAL_H__ */
