/*
 *
 * Copyright 2020-present Facebook. All Rights Reserved.
 *
 * This file contains code to support IPMI2.0 Specificaton available @
 * http://www.intel.com/content/www/us/en/servers/ipmi/ipmi-specifications.html
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
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openbmc/libgpio.h>
#include <openbmc/obmc-i2c.h>
#include <facebook/fbgc_gpio.h>
#include "fbgc_common.h"
#include <openbmc/kv.h>

// Platform signature of image: Grand Canyon
const char platform_signature[PLAT_SIG_SIZE] = {0x47, 0x72, 0x61, 0x6e, 0x64, 0x20, 0x43, 0x61, 0x6e, 0x79, 0x6f, 0x6e, 0x20, 0x20, 0x20, 0x20};

const char* board_stage[] = {"Pre EVT", "EVT", "DVT", "PVT", "MP"};

static int
get_chassis_type_by_gpio(uint8_t *type) {
  int chassis_type_value = 0;
  gpio_value_t uic_loc_type_in = GPIO_VALUE_INVALID;
  gpio_value_t uic_rmt_type_in = GPIO_VALUE_INVALID;
  gpio_value_t scc_loc_type_0 = GPIO_VALUE_INVALID;
  gpio_value_t scc_rmt_type_0 = GPIO_VALUE_INVALID;

  uic_loc_type_in = gpio_get_value_by_shadow(fbgc_get_gpio_name(GPIO_UIC_LOC_TYPE_IN));
  uic_rmt_type_in = gpio_get_value_by_shadow(fbgc_get_gpio_name(GPIO_UIC_RMT_TYPE_IN));
  scc_loc_type_0  = gpio_get_value_by_shadow(fbgc_get_gpio_name(GPIO_SCC_LOC_TYPE_0));
  scc_rmt_type_0  = gpio_get_value_by_shadow(fbgc_get_gpio_name(GPIO_SCC_RMT_TYPE_0));

  if ((uic_loc_type_in == GPIO_VALUE_INVALID) || (uic_rmt_type_in == GPIO_VALUE_INVALID) ||
      (scc_loc_type_0 == GPIO_VALUE_INVALID)  || (scc_rmt_type_0 == GPIO_VALUE_INVALID)) {
    syslog(LOG_ERR, "%s() failed to get chassis type gpio", __func__);
    return -1;
  }

  //                 UIC_LOC_TYPE_IN   UIC_RMT_TYPE_IN   SCC_LOC_TYPE_0   SCC_RMT_TYPE_0
  // Type 5                        0                 0                0                0
  // Type 7 Headnode               0                 1                0                1

  chassis_type_value = CHASSIS_TYPE_BIT_3(uic_loc_type_in) | CHASSIS_TYPE_BIT_2(uic_rmt_type_in) |
                       CHASSIS_TYPE_BIT_1(scc_loc_type_0)  | CHASSIS_TYPE_BIT_0(scc_rmt_type_0);

  if (chassis_type_value == CHASSIS_TYPE_5_VALUE) {
    *type = CHASSIS_TYPE5;
  } else if (chassis_type_value == CHASSIS_TYPE_7_VALUE) {
    *type = CHASSIS_TYPE7;
  } else {
    syslog(LOG_ERR, "%s() Unknown chassis type.", __func__);
    return -1;
  }

  return 0;
}

int
fbgc_common_get_chassis_type(uint8_t *type) {
  char system_info[MAX_VALUE_LEN] = {0};
  int sku_value = 0;
  int ret = 0;

  if (type == NULL) {
    syslog(LOG_ERR, "%s(): Failed to get chassis type because of NULL parameter\n", __func__);
    return -1;
  }

  ret = kv_get(SYSTEM_INFO, system_info, NULL, KV_FPERSIST);

  if (ret < 0) { //cache not ready, get from gpio directly
    if (get_chassis_type_by_gpio(type) < 0) {
      syslog(LOG_ERR, "%s(): Unknown chassis type.\n", __func__);
      goto error;
    } else {
      return 0;
    }
  }

  sku_value = atoi(system_info);

  if (sku_value >= MAX_SKU_VALUE) {
    syslog(LOG_WARNING, "%s(): Failed to get chassis type because SKU value exceed limit, value: %d\n", __func__, sku_value);
    goto error;
  }

  //  SKU[5:0] = {UIC_ID0, UIC_ID1, UIC_TYPE0, UIC_TYPE1, UIC_TYPE2, UIC_TYPE3}
  switch ((sku_value & 0xF)) {
    case CHASSIS_TYPE_5_VALUE:
      *type = CHASSIS_TYPE5;
      break;
    case CHASSIS_TYPE_7_VALUE:
      *type = CHASSIS_TYPE7;
      break;
    default:
      syslog(LOG_WARNING, "%s(): Failed to get chassis type because SKU value is wrong, value: %d\n", __func__, sku_value);
      goto error;
  }

  return 0;

error:
  syslog(LOG_ERR, "%s(): Using default chassis type: Type5\n", __func__);
  *type = CHASSIS_TYPE5;
  return 0;
}

void
msleep(int msec) {
  struct timespec req;

  req.tv_sec = 0;
  req.tv_nsec = msec * 1000 * 1000;

  while(nanosleep(&req, &req) == -1 && errno == EINTR) {
    continue;
  }
}

int
fbgc_common_server_stby_pwr_sts(uint8_t *val) {
  gpio_value_t pg_gpio = GPIO_VALUE_LOW;

  if (val == NULL) {
    syslog(LOG_WARNING, "%s() NULL pointer: *val", __func__);
    return -1;
  }

  pg_gpio = gpio_get_value_by_shadow(fbgc_get_gpio_name(GPIO_COMP_STBY_PG_IN));
  if (pg_gpio == GPIO_VALUE_INVALID) {
    syslog(LOG_WARNING, "%s() Can not get 12V power status via GPIO pin", __func__);
    return -1;
  } else if (pg_gpio == GPIO_VALUE_HIGH) {
    *val = STAT_12V_ON;
  } else {
    *val = STAT_12V_OFF;
  }

  return 0;
}

uint8_t
cal_crc8(uint8_t crc, uint8_t const *data, uint8_t len) {
  uint8_t const *end = data + len;
  static uint8_t const crc8_table[] =
  { 0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3};

  if (NULL == data) {
    return 0;
  }

  crc &= 0xff;

  while (data < end) {
    crc = crc8_table[crc ^ *data++];
  }

  return crc;
}

uint8_t
hex_c2i(const char c) {
  if (c && (c >= '0') && (c <= '9')) {
    return (c - '0');
  }
  if (c && (c >= 'A') && (c <= 'F')) {
    return (c - 'A' + 10);
  }
  if (c && (c >= 'a') && (c <= 'f')) {
    return (c - 'a' + 10);
  }

  return 0xFF;
}

int
string_2_byte(const char* c) {
  uint8_t h_nibble = hex_c2i(c[0]);
  if (h_nibble > 0xF) {
    return -1;
  }
  uint8_t l_nibble = hex_c2i(c[1]);
  if (l_nibble > 0xF) {
    return -1;
  }

  return (h_nibble << 4) | (l_nibble);
}

bool
start_with(const char *s, const char *p) {
  if ((0 == s) || (0 == p)) {
    return false;
  }
  while (*p && *s && (*p == *s)) {
    p++;
    s++;
  }

  return (*p == '\0');
}

int
split(char **dst, char *src, char *delim, int max_size) {
  char *s = strtok(src, delim);
  int size = 0;

  while ((NULL != s) && (size < max_size) ) {
    *dst++ = s;
    size++;
    s = strtok(NULL, delim);
  }

  return (size == max_size) ? -1 : size;
}

int
fbgc_common_get_system_stage(uint8_t *stage) {
  if (stage == NULL) {
    syslog(LOG_WARNING, "%s(): system stage is missing", __func__);
  }
  uint8_t board_id[BOARD_ID_PIN_NUM] = 
        {GPIO_BOARD_REV_ID2, GPIO_BOARD_REV_ID1, GPIO_BOARD_REV_ID0};
  int i = 0;
  int val = 0;

  *stage = 0;
  for (i = 0; i < BOARD_ID_PIN_NUM; i++) {
    val = gpio_get_value_by_shadow(fbgc_get_gpio_name(board_id[i]));
    if (val == GPIO_VALUE_INVALID) {
      syslog(LOG_WARNING, "%s(): failed to get board ID", __func__);
      return -1;
    }
    *stage = (*stage << 1) + val;
  }

  return 0;
}

int
check_image_md5(const char* image_path, int cal_size, uint32_t md5_offset) {
  int fd = 0, sum = 0, byte_num = 0 , ret = 0, read_bytes = 0;
  unsigned char read_buf[MD5_READ_BYTES] = {0};
  unsigned char md5_digest[MD5_DIGEST_LENGTH] = {0};
  EVP_MD_CTX *context = NULL;

  if (image_path == NULL) {
    syslog(LOG_WARNING, "%s(): failed to calculate MD5 due to NULL parameters.", __func__);
    return -1;
  }

  if (cal_size <= 0) {
    syslog(LOG_WARNING, "%s(): failed to calculate MD5 due to wrong calculate size: %d.", __func__, cal_size);
    return -1;
  }

  if (md5_offset <= 0) {
    syslog(LOG_WARNING, "%s(): failed to calculate MD5 due to wrong offset of MD5: 0x%X.", __func__, md5_offset);
    return -1;
  }

  fd = open(image_path, O_RDONLY);

  if (fd < 0) {
    syslog(LOG_WARNING, "%s(): failed to open %s to calculate MD5.", __func__, image_path);
    return -1;
  }

  lseek(fd, 0, SEEK_SET);

  context = EVP_MD_CTX_create();
  if (!context) {
    syslog(LOG_WARNING, "%s(): failed to initialize message digest context.", __func__);
    ret = -1;
    goto exit;
  }

  if (!EVP_DigestInit_ex(context, EVP_md5(), NULL)) {
    syslog(LOG_WARNING, "%s(): failed to initialize MD5 context.", __func__);
    ret = -1;
    goto exit;
  }

  while (sum < cal_size) {
    read_bytes = MD5_READ_BYTES;
    if ((sum + MD5_READ_BYTES) > cal_size) {
      read_bytes = cal_size - sum;
    }

    byte_num = read(fd, read_buf, read_bytes);
    if (!EVP_DigestUpdate(context, read_buf, byte_num)) {
      syslog(LOG_WARNING, "%s(): failed to update context to calculate MD5 of %s.", __func__, image_path);
      ret = -1;
      goto exit;
    }
    sum += byte_num;
  }

  if (!EVP_DigestFinal_ex(context, md5_digest, NULL)) {
    syslog(LOG_WARNING, "%s(): failed to calculate MD5 of %s.", __func__, image_path);
    ret = -1;
    goto exit;
  }

  memset(read_buf, 0, sizeof(read_buf));
  lseek(fd, md5_offset, SEEK_SET);
  byte_num = read(fd, read_buf, MD5_DIGEST_LENGTH);

  if (byte_num != MD5_DIGEST_LENGTH) {
    syslog(LOG_WARNING, "%s(): failed to read the signature of %s.", __func__, image_path);
    ret = -1;
    goto exit;
  }

  if (memcmp(md5_digest, read_buf, MD5_DIGEST_LENGTH) != 0) {
    ret = -1;
    goto exit;
  }

exit:
  EVP_MD_CTX_destroy(context);
  close(fd);
  return ret;
}

int
check_image_signature(const char* image_path, uint32_t sig_offset) {
  int fd = 0, ret = 0, byte_num = 0;
  char read_buf[PLAT_SIG_SIZE] = {0};

  if ((image_path == NULL)) {
   syslog(LOG_WARNING, "%s(): failed to check platform signature of image due to NULL parameter.", __func__);
    ret = -1;
    goto exit;
  }

  if (sig_offset <= 0) {
    syslog(LOG_WARNING, "%s(): failed to check platform signature of %s due to wrong offset: 0x%X", __func__, image_path, sig_offset);
    ret = -1;
    goto exit;
  }

  fd = open(image_path, O_RDONLY);

  if (fd < 0 ) {
    syslog(LOG_WARNING, "%s(): failed to open %s to check platform signature.", __func__, image_path);
    ret = -1;
    goto exit;
  }

  lseek(fd, sig_offset, SEEK_SET);
  byte_num = read( fd, read_buf, sizeof(read_buf));

  if (byte_num != sizeof(read_buf)) {
    syslog(LOG_WARNING, "%s(): failed to check platform signature of %s because read failed.", __func__, image_path);
    ret = -1;
    goto exit;
  }

  if (strncmp(platform_signature, read_buf, sizeof(read_buf)) != 0) {
    ret = -1;
  }

exit:
  close(fd);
  return ret;
}

int
get_server_board_revision_id(uint8_t* board_rev_id, uint8_t board_rev_id_len) {
  int i2cfd = 0, ret = 0, retry = 0;
  uint8_t tbuf[1] = {0};
  uint8_t tlen = 0;
  FILE* fp = NULL;
  char buf[MAX_SYS_CMD_RESP_LEN] = {0};

  if (board_rev_id == NULL) {
    syslog(LOG_WARNING, "%s(): fail to get board revision id due to NULL parameter", __func__);
    return -1;
  }

  i2cfd = i2c_cdev_slave_open(I2C_BS_FPGA_BUS, BS_FPGA_SLAVE_ADDR >> 1, I2C_SLAVE_FORCE_CLAIM);
  if (i2cfd < 0) {
    syslog(LOG_WARNING, "%s(): fail to open device: I2C BUS: %d", __func__, I2C_BS_FPGA_BUS);
    return i2cfd;
  }

  tbuf[0] = BS_FPGA_BOARD_REV_ID_OFFSET;
  tlen = sizeof(tbuf);

  while (retry < MAX_RETRY) {
    ret = i2c_rdwr_msg_transfer(i2cfd, BS_FPGA_SLAVE_ADDR, tbuf, tlen, board_rev_id, board_rev_id_len);
    if ( ret < 0 ) {
      retry++;
      msleep(100);
    } else {
      ret = 0;
      break;
    }
  }

  if (retry == MAX_RETRY) {
    syslog(LOG_WARNING, "%s(): fail to read server FPGA offset: 0x%02X via i2c\n", __func__, BS_FPGA_BOARD_REV_ID_OFFSET);
    ret = -1;
    goto exit;
  }

  // Add this workaround to handle the wrong BS Rev ID on PVT Barton Springs.
  if (*board_rev_id == (STAGE_DVT + 1)) {
    if ((fp = popen("/usr/local/bin/fruid-util server | grep 'Product Version' | grep 'PVT'", "r")) == NULL) {
      syslog(LOG_WARNING, "%s(): fail to get Product Version of Bartion Springs\n", __func__);
      ret = -1;
      goto exit;
    }
    if (fgets(buf, sizeof(buf), fp) != NULL) {
      *board_rev_id = (STAGE_PVT + 1);
    }
  }
  pclose(fp);

exit:
  close(i2cfd);
  return ret;
}

int
fbgc_common_dev_id(char *str, uint8_t *dev) {
  if ((str == NULL) || (dev == NULL)) {
    syslog(LOG_WARNING, "%s(): Failed to get device id due to NULL parameter", __func__);
    return -1;
  }

  if (strcmp(str, "e1s0") == 0) {
    *dev = DEV_ID0_E1S;
  } else if (strcmp(str, "e1s1") == 0) {
    *dev = DEV_ID1_E1S;
  } else {
#ifdef DEBUG
    syslog(LOG_WARNING, "s%(): Wrong device name: %s", __func__, str);
#endif
    return -1;
  }

  return 0;
}
