/*
 *
 * Copyright 2019-present Facebook. All Rights Reserved.
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
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <syslog.h>
#include <openbmc/libgpio.h>
#include <openbmc/obmc-i2c.h>
#include <openbmc/pex88000.h>
#include <switchtec/switchtec.h>
#include <switchtec/gas.h>
#include "pal.h"

#define PAX_I2C_LOCK "/tmp/pax_lock"

#define MRPC_PART_INFO_GET_ALL_INFO_EX 0x3

static int pax_lock()
{
  int fd;

  fd = open(PAX_I2C_LOCK, O_RDONLY | O_CREAT, 0666);
  if (fd < 0)
    return -1;

  if (flock(fd, LOCK_EX) < 0) {
    syslog(LOG_WARNING, "%s: errno = %d", __func__, errno);
    return -1;
  }

  return fd;
}

static int pax_unlock(int fd)
{
  if (fd < 0)
    return -1;

  if (flock(fd, LOCK_UN) < 0) {
    syslog(LOG_WARNING, "%s: errno = %d", __func__, errno);
  }

  close(fd);
  return 0;
}

int pal_check_pax_fw_type(uint8_t comp, const char *fwimg)
{
  int fd;
  struct switchtec_fw_image_info info;

  if (pal_check_switch_config())
    return -2;

  fd = open(fwimg, O_RDONLY);
  if (fd < 0 || switchtec_fw_file_info(fd, &info) < 0)
    return -1;

  close(fd);

  if ((comp == PAX_BL2 && info.type == SWITCHTEC_FW_TYPE_BL2) ||
      (comp == PAX_IMG && info.type == SWITCHTEC_FW_TYPE_IMG) ||
      (comp == PAX_CFG && info.type == SWITCHTEC_FW_TYPE_CFG) ) {
    return 0;
  } else {
    return -1;
  }
}

void pal_clear_pax_cache(uint8_t paxid)
{
  char key[MAX_KEY_LEN];

  snprintf(key, sizeof(key), "pax%d_bl2", paxid);
  kv_del(key, 0);
  snprintf(key, sizeof(key), "pax%d_img", paxid);
  kv_del(key, 0);
  snprintf(key, sizeof(key), "pax%d_cfg", paxid);
  kv_del(key, 0);
}

int pal_pax_fw_update(uint8_t paxid, const char *fwimg)
{
  int fd, ret;
  char cmd[128] = {0};

  if (pal_is_server_off())
    return -1;

  snprintf(cmd, sizeof(cmd), SWITCHTEC_UPDATE_CMD, SWITCH_BASE_ADDR + paxid, fwimg);
  fd = pax_lock();
  if (fd < 0)
    return -1;

  ret = system(cmd);
  pax_unlock(fd);
  if (WIFEXITED(ret) && (WEXITSTATUS(ret) == 0)) {
    return 0;
  } else {
    return -1;
  }
}

int pal_read_pax_dietemp(uint8_t sensor_num, float *value)
{
  int fd, ret = 0;
  uint8_t addr;
  uint8_t paxid = sensor_num - MB_SWITCH_PAX0_DIE_TEMP;
  uint32_t temp, sub_cmd_id;
  char device_name[LARGEST_DEVICE_NAME] = {0};
  struct switchtec_dev *dev;

  if (pal_is_server_off())
    return ERR_SENSOR_NA;

  addr = SWITCH_BASE_ADDR + paxid;
  snprintf(device_name, LARGEST_DEVICE_NAME, SWITCHTEC_DEV, addr);

  fd = pax_lock();
  if (fd < 0)
    return ERR_SENSOR_NA;

  dev = switchtec_open(device_name);
  if (dev == NULL) {
    syslog(LOG_WARNING, "%s: switchtec open %s failed", __func__, device_name);
    pax_unlock(fd);
    return ERR_SENSOR_NA;
  }

  sub_cmd_id = MRPC_DIETEMP_GET_GEN4;
  ret = switchtec_cmd(dev, MRPC_DIETEMP, &sub_cmd_id,
                      sizeof(sub_cmd_id), &temp, sizeof(temp));
  switchtec_close(dev);
  pax_unlock(fd);

  if (ret == 0) {
    *value = (float) temp / 100.0;
    return 0;
  } else {
    return ERR_SENSOR_NA;
  }
}

struct switchtec_flash_info_gen4_Ex {
  uint32_t firmware_version;
  uint32_t flash_size;
  uint16_t device_id;
  uint8_t ecc_enable;
  uint8_t rsvd1;
  uint8_t running_bl2_flag;
  uint8_t running_cfg_flag;
  uint8_t running_img_flag;
  uint8_t running_key_flag;
  uint8_t redundancy_key_flag;
  uint8_t redundancy_bl2_flag;
  uint8_t redundancy_cfg_flag;
  uint8_t redundancy_img_flag;
  uint32_t rsvd2[11];
  struct switchtec_flash_part_info_gen4 {
    uint32_t image_crc;
    uint32_t image_len;
    uint32_t image_version;
    uint8_t valid;
    uint8_t active;
    uint8_t read_only;
    uint8_t is_using;
    uint32_t part_start;
    uint32_t part_end;
    uint32_t part_offset;
    uint32_t part_size_dw;
  } map0, map1, keyman0, keyman1, bl20, bl21, cfg0, cfg1, img0, img1, nvlog, vendor[8];
};

static int get_pax_ver(uint8_t paxid, uint8_t type, char *ver)
{
  int fd, ret;
  uint8_t subcmd;
  uint16_t* p;
  char device_name[LARGEST_DEVICE_NAME] = {0};
  struct switchtec_dev *dev;
  struct switchtec_flash_info_gen4_Ex all_info;
  struct switchtec_flash_part_info_gen4 *info_0, *info_1;
  uint8_t tbuf[4] = {0x04, 0x00, 0x3c, 0x83}; //Read CFG ver reg
  uint8_t rbuf[4] = {0};
  char device[64] = {0};
  uint8_t addr = 0xb2;
  uint8_t bus;

  if (pal_is_server_off())
    return -1;

  if (pal_check_switch_config()) {

    switch (paxid) {
      case 0:
        bus = 19;
      break;
      case 1:
        bus = 20;
      break;
      case 2:
        bus = 21;
      break;
      case 3:
        bus = 22;
      break;

      default:
        return -1;
    }

    snprintf(device, sizeof(device), "/dev/i2c-%d", bus);
    fd = open(device, O_RDWR);
    if (fd < 0) {
      close(fd);
      return -1;
    }

    ret = i2c_rdwr_msg_transfer(fd, addr, tbuf, 4, rbuf, 4);

    snprintf(ver, MAX_VALUE_LEN, "0x%02X%02X%02X%02X",
   	     rbuf[0], rbuf[1], rbuf[2], rbuf[3]);

    close(fd);
  } else {
    snprintf(device_name, LARGEST_DEVICE_NAME, SWITCHTEC_DEV, SWITCH_BASE_ADDR + paxid);
   
    fd = pax_lock();
    if (fd < 0)
      return -1;
   
    dev = switchtec_open(device_name);
    if (dev == NULL) {
      syslog(LOG_WARNING, "%s: switchtec open %s failed", __func__, device_name);
      pax_unlock(fd);
      return -1;
    }
   
    subcmd = MRPC_PART_INFO_GET_ALL_INFO_EX;
    ret = switchtec_cmd(dev, MRPC_PART_INFO, &subcmd,
                        sizeof(subcmd), &all_info, sizeof(all_info));
   
    switchtec_close(dev);
    pax_unlock(fd);
   
    if (ret == 0) {
      switch (type) {
        case SWITCHTEC_FW_TYPE_BL2:
   	info_0 = &(all_info.bl20);
   	info_1 = &(all_info.bl21);
   	break;
        case SWITCHTEC_FW_TYPE_IMG:
   	info_0 = &(all_info.img0);
   	info_1 = &(all_info.img1);
   	break;
        default:
   	return -1;
      };
      if (info_0->is_using)
        p = (uint16_t*)&info_0->image_version;
      else
        p = (uint16_t*)&info_1->image_version;
   
      snprintf(ver, MAX_VALUE_LEN, "%X.%02X B%03X",
   	     p[1] >> 8, p[1] & 0xFF, p[0] & 0xFFF);
    } else {
      syslog(LOG_WARNING, "%s: switchtec get %s version failed", __func__, device_name);
    }
  }

  return ret;
}

int pal_get_pax_bl2_ver(uint8_t paxid, char *ver)
{
  return get_pax_ver(paxid, SWITCHTEC_FW_TYPE_BL2, ver);
}

int pal_get_pax_fw_ver(uint8_t paxid, char *ver)
{
  return get_pax_ver(paxid, SWITCHTEC_FW_TYPE_IMG, ver);
}

int pal_get_pex_therm(uint8_t sensor_num, float *value) {
  int fd, fd_lock;
  char dev[64] = {0};
  uint8_t tbuf[][16] = {
    {0x03, 0x00, 0x3c, 0xb3, 0x00, 0x00, 0x00, 0x07}, // Set full address
    {0x03, 0x58, 0x3c, 0x40, 0xff, 0xe7, 0x85, 0x04}, // Set addr 0xffe78504
    {0x03, 0x58, 0x3c, 0x41, 0x20, 0x06, 0x53, 0xe8}, // Set data 0x200653e8
    {0x03, 0x58, 0x3c, 0x42, 0x00, 0x00, 0x00, 0x01}, // Write data
    {0x03, 0x58, 0x3c, 0x40, 0xff, 0xe7, 0x85, 0x38}, // Set addr 0xffe78538
    {0x03, 0x58, 0x3c, 0x42, 0x00, 0x00, 0x00, 0x02}, // Read data
    {0x04, 0x58, 0x3c, 0x41},                         // Get response
  };
  uint8_t rbuf[8] = {0};
  uint8_t addr = 0xb2;
  uint8_t bus;
  uint16_t temp128;
  switch (sensor_num) {
    case MB_SENSOR_PEX0_THERM_REMOTE:
      bus = 19;
    break;
    case MB_SENSOR_PEX1_THERM_REMOTE:
      bus = 20;
    break;
    case MB_SENSOR_PEX2_THERM_REMOTE:
      bus = 21;
    break;
    case MB_SENSOR_PEX3_THERM_REMOTE:
      bus = 22;
    break;
    default:
      return ERR_SENSOR_NA;
  }
  snprintf(dev, sizeof(dev), "/dev/i2c-%d", bus);
  fd = open(dev, O_RDWR);
  if (fd < 0) {
    close(fd);
    return ERR_SENSOR_NA;
  }
  fd_lock = pax_lock();
  //Send cmd to Axis reg
  for (int i = 0; i < 6; i++) {
    if (i2c_rdwr_msg_transfer(fd, addr, tbuf[i], 8, NULL, 0) != 0) {
      syslog(LOG_WARNING, "PEX THERM: sending cmd%d to axis reg of bus %d failed\n", i, bus);
      close(fd);
      pax_unlock(fd_lock);
      return ERR_SENSOR_NA;
    }
  }
  //Get response
  int ret = i2c_rdwr_msg_transfer(fd, addr, tbuf[6], 4, rbuf, 4);
  if (ret != 0) {
    syslog(LOG_WARNING, "PEX THERM: getting response to axis reg of bus %d failed\n", bus);
    ret = ERR_SENSOR_NA;
  }
  temp128 = (rbuf[2] << 8) | rbuf[3];
  *value = (float)temp128/128;
  close(fd);
  pax_unlock(fd_lock);

  return ret;
}

int pal_paxid_to_brcm_bus(uint8_t paxid) {
  int bus = 0;

  switch (paxid) {
    case 0:
      bus = 19;
      break;
    case 1:
      bus = 20;
      break;
    case 2:
      bus = 21;
      break;
    case 3:
      bus = 22;
      break;

    default:
      return -1;
    }

  return bus;
}

int pal_print_pex_ver(uint8_t paxid) {
  int fd, bus, ret;
  char sbr_ver[MAX_VALUE_LEN] = {0};
  char cfg_ver[MAX_VALUE_LEN] = {0};
  char main_ver[MAX_VALUE_LEN] = {0};
  char sbr_ver_key[MAX_VALUE_LEN] = {0};
  char cfg_ver_key[MAX_VALUE_LEN] = {0};
  char main_ver_key[MAX_VALUE_LEN] = {0};

  if (pal_is_server_off()) {
    printf("PEX%d SBR UTP Version: NA\n", paxid);
    printf("PEX%x MFG CFG Version: NA\n", paxid);
    printf("PEX%x Main FW Version: NA\n", paxid);
    return -1;
  }

  bus = pal_paxid_to_brcm_bus(paxid);

  snprintf(sbr_ver_key, sizeof(sbr_ver_key), "sbr_version_%d", paxid);
  snprintf(cfg_ver_key, sizeof(cfg_ver_key), "mfg_version_%d", paxid);
  snprintf(main_ver_key, sizeof(main_ver_key), "main_version_%d", paxid);

  fd = pax_lock();
  if (kv_get(sbr_ver_key, sbr_ver, NULL, 0)) {
    ret = pex88000_get_sbr_version(bus, sbr_ver);
    if (ret < 0) {
      printf("PEX%d SBR UTP Version: NA\n", paxid);
    } else {
      printf("PEX%d SBR UTP Version: %s\n", paxid, sbr_ver);
      kv_set(sbr_ver_key, sbr_ver, 0, 0);
    }
  } else {
    printf("PEX%d SBR UTP Version: %s\n", paxid, sbr_ver);
  }

  if (kv_get(cfg_ver_key, cfg_ver, NULL, 0)) {
    ret = pex88000_get_MFG_config_version(bus, cfg_ver);
    if (ret < 0) {
      printf("PEX%x MFG CFG Version: NA\n", paxid);
    } else {
      printf("PEX%x MFG CFG Version: %s\n", paxid, cfg_ver);
      kv_set(cfg_ver_key, cfg_ver, 0, 0);
    }
  } else {
    printf("PEX%x MFG CFG Version: %s\n", paxid, cfg_ver);
  }

  if (kv_get(main_ver_key, main_ver, NULL, 0)) {
    ret = pex88000_get_main_version(bus, main_ver);
    if (ret < 0) {
      printf("PEX%x Main FW Version: NA\n", paxid);
    } else {
      printf("PEX%x Main FW Version: %s\n", paxid, main_ver);
      kv_set(main_ver_key, main_ver, 0, 0);
    }
  } else {
    printf("PEX%x Main FW Version: %s\n", paxid, main_ver);
  }
  pax_unlock(fd);

  return 0;
}

int pal_get_pax_cfg_ver(uint8_t paxid, char *ver)
{
  int fd, ret = 0;
  char device_name[LARGEST_DEVICE_NAME] = {0};
  struct switchtec_dev *dev;
  size_t map_size;
  unsigned int x;
  gasptr_t map;

  if (pal_is_server_off())
    return -1;

  if (pal_check_switch_config()) {
    return get_pax_ver(paxid, SWITCHTEC_FW_TYPE_CFG, ver);
  } else {
    snprintf(device_name, LARGEST_DEVICE_NAME, SWITCHTEC_DEV, SWITCH_BASE_ADDR + paxid);
   
    fd = pax_lock();
    if (fd < 0)
      return -1;
   
    dev = switchtec_open(device_name);
    if (dev == NULL) {
      syslog(LOG_WARNING, "%s: switchtec open %s failed", __func__, device_name);
      pax_unlock(fd);
      return -1;
    }
   
    map = switchtec_gas_map(dev, 0, &map_size);
    if (map != SWITCHTEC_MAP_FAILED) {
      ret = gas_read32(dev, (void __gas*)map + 0x2104, &x);
      switchtec_gas_unmap(dev, map);
    } else {
      ret = -1;
    }
   
    switchtec_close(dev);
    pax_unlock(fd);
   
    if (ret == 0)
      snprintf(ver, MAX_VALUE_LEN, "%X", x);
  }
  return ret < 0? -1: 0;
}
