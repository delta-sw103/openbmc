/*
 * bic-util
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <stdint.h>
#include <pthread.h>
#include <ctype.h>
#include <facebook/bic.h>
#include <facebook/bic_bios_fwupdate.h>
#include <openbmc/ipmi.h>
#include <openbmc/pal.h>
#include <facebook/fby35_common.h>
#include <facebook/fby35_gpio.h>
#include <sys/time.h>
#include <time.h>

static uint8_t bmc_location = 0xff;
static const char *intf_name[4] = {"Server Board", "Front Expansion Board", "Riser Expansion Board", "Baseboard"};
const uint8_t intf_size = 4;

static const char *option_list[] = {
  "--get_gpio",
  "--set_gpio [gpio_num] [value]",
  "--check_status",
  "--get_dev_id",
  "--reset",
  "--get_post_code",
  "--get_sdr",
  "--read_sensor",
  "--perf_test [loop_count] (0 to run forever)",
  "--clear_cmos",
  "--file [path]",
  "--check_usb_port [sb|1ou|2ou]",
};

static void
print_usage_help(void) {
  size_t i;

  printf("Usage: bic-util <%s> <[0..n]data_bytes_to_send>\n", slot_usage);
  printf("Usage: bic-util <%s> <option>\n", slot_usage);
  printf("       option:\n");
  for (i = 0; i < sizeof(option_list)/sizeof(option_list[0]); i++)
    printf("       %s\n", option_list[i]);
}

// Check BIC status
static int
util_check_status(uint8_t slot_id) {
  int ret = 0;
  uint8_t status;

  // BIC status is only valid if 12V-on. check this first
  ret = pal_get_server_12v_power(slot_id, &status);
  if ( (ret < 0) || (SERVER_12V_OFF == status) ) {
    ret = pal_is_fru_prsnt(slot_id, &status);

    if (ret < 0) {
       printf("unable to check BIC status\n");
       return ret;
    }

    if (status == 0) {
      printf("Slot is empty, unable to check BIC status\n");
    } else {
      printf("Slot is 12V-off, unable to check BIC status\n");
    }
    ret = 0;
  } else {
    if ( is_bic_ready(slot_id, NONE_INTF) == BIC_STATUS_SUCCESS ) {
      printf("BIC status ok\n");
      ret = 0;
    } else {
      printf("Error: BIC not ready\n");
      ret = -1;
    }
  }
  return ret;
}

// Test to Get device ID
static int
util_get_device_id(uint8_t slot_id) {
  int ret = 0;
  ipmi_dev_id_t id = {0};

  ret = bic_get_dev_id(slot_id, &id, NONE_INTF);
  if (ret) {
    printf("util_get_device_id: bic_get_dev_id returns %d\n", ret);
    return ret;
  }

  // Print response
  printf("Device ID: 0x%X\n", id.dev_id);
  printf("Device Revision: 0x%X\n", id.dev_rev);
  printf("Firmware Revision: 0x%X:0x%X\n", id.fw_rev1, id.fw_rev2);
  printf("IPMI Version: 0x%X\n", id.ipmi_ver);
  printf("Device Support: 0x%X\n", id.dev_support);
  printf("Manufacturer ID: 0x%X:0x%X:0x%X\n", id.mfg_id[2], id.mfg_id[1], id.mfg_id[0]);
  printf("Product ID: 0x%X:0x%X\n", id.prod_id[1], id.prod_id[0]);
  printf("Aux. FW Rev: 0x%X:0x%X:0x%X:0x%X\n", id.aux_fw_rev[0], id.aux_fw_rev[1],id.aux_fw_rev[2],id.aux_fw_rev[3]);

  return ret;
}

// reset BIC
static int
util_bic_reset(uint8_t slot_id) {
  int ret = 0;
  ret = bic_reset(slot_id);
  printf("Performing BIC reset, status %d\n", ret);
  return ret;
}

static int
util_is_numeric(char **argv) {
  int j = 0;
  int len = 0;
  for (int i = 0; i < 2; i++) { //check netFn cmd
    len = strlen(argv[i]);
    if (len > 2 && argv[i][0] == '0' && (argv[i][1] == 'x' || argv[i][1] == 'X')) {
      j=2;
      for (; j < len; j++) {
        if (!isxdigit(argv[i][j]))
          return 0;
      }
    } else {
      j=0;
      for (; j < len; j++) {
        if (!isdigit(argv[i][j]))
          return 0;
      }
    }
  }
  return 1;
}

static int
process_command(uint8_t slot_id, int argc, char **argv) {
  int i, ret, retry = 2;
  uint8_t tbuf[256] = {0x00};
  uint8_t rbuf[256] = {0x00};
  uint8_t tlen = 0;
  uint8_t rlen = 0;

  for (i = 0; i < argc; i++) {
    tbuf[tlen++] = (uint8_t)strtoul(argv[i], NULL, 0);
  }

  while (retry >= 0) {
    ret = bic_ipmb_wrapper(slot_id, tbuf[0]>>2, tbuf[1], &tbuf[2], tlen-2, rbuf, &rlen);
    if (ret == 0)
      break;

    retry--;
  }
  if (ret) {
    printf("BIC no response!\n");
    return ret;
  }

  for (i = 0; i < rlen; i++) {
    if (!(i % 16) && i)
      printf("\n");

    printf("%02X ", rbuf[i]);
  }
  printf("\n");

  return 0;
}

static int
process_file(uint8_t slot_id, char *path) {
#define MAX_ARG_NUM 64
  FILE *fp;
  int argc;
  char buf[1024];
  char *str, *next, *del=" \n";
  char *argv[MAX_ARG_NUM] = {NULL};

  if (!(fp = fopen(path, "r"))) {
    syslog(LOG_WARNING, "Failed to open %s", path);
    return -1;
  }

  while (fgets(buf, sizeof(buf), fp) != NULL) {
    str = strtok_r(buf, del, &next);
    for (argc = 0; argc < MAX_ARG_NUM && str; argc++, str = strtok_r(NULL, del, &next)) {
      if (str[0] == '#')
        break;

      if (!argc && !strcmp(str, "echo")) {
        printf("%s", (*next) ? next : "\n");
        break;
      }
      argv[argc] = str;
    }
    if (argc <= 1) {
      continue;
    }

    process_command(slot_id, argc, argv);
  }
  fclose(fp);

  return 0;
}

static int
util_get_gpio(uint8_t slot_id) {
  int ret = 0;
  uint8_t i;
  uint8_t gpio_pin_cnt = y35_get_gpio_list_size(slot_id);
  char gpio_pin_name[32] = "\0";
  bic_gpio_t gpio = {0};

  ret = bic_get_gpio(slot_id, &gpio, NONE_INTF);
  if ( ret < 0 ) {
    printf("%s() bic_get_gpio returns %d\n", __func__, ret);
    return ret;
  }

  // Print the gpio index, name and value
  for (i = 0; i < gpio_pin_cnt; i++) {
    y35_get_gpio_name(slot_id, i, gpio_pin_name);
    printf("%d %s: %d\n",i , gpio_pin_name, BIT_VALUE(gpio, i));
  }

  return ret;
}

static int
util_set_gpio(uint8_t slot_id, uint8_t gpio_num, uint8_t gpio_val) {
  uint8_t gpio_pin_cnt = y35_get_gpio_list_size(slot_id);
  char gpio_pin_name[32] = "\0";
  int ret = -1;

  if ( gpio_num > gpio_pin_cnt ) {
    printf("slot %d: Invalid GPIO pin number %d\n", slot_id, gpio_num);
    return ret;
  }

  y35_get_gpio_name(slot_id, gpio_num, gpio_pin_name);
  printf("slot %d: setting [%d]%s to %d\n", slot_id, gpio_num, gpio_pin_name, gpio_val);

  ret = bic_set_gpio(slot_id, gpio_num, gpio_val);
  if (ret < 0) {
    printf("%s() bic_set_gpio returns %d\n", __func__, ret);
  }

  return ret;
}

static int
util_perf_test(uint8_t slot_id, int loopCount) {
#define NUM_SLOTS FRU_SLOT4
  enum cmd_profile_type {
    CMD_AVG_DURATION = 0x0,
    CMD_MIN_DURATION,
    CMD_MAX_DURATION,
    CMD_NUM_SAMPLES,
    CMD_PROFILE_NUM
  };

  long double cmd_profile[NUM_SLOTS][CMD_PROFILE_NUM] = {0};
  int ret = 0;
  ipmi_dev_id_t id = {0};
  int i = 0;
  int index = slot_id -1;
  long double elapsedTime = 0;

  cmd_profile[index][CMD_MIN_DURATION] = 3000000;
  cmd_profile[index][CMD_MAX_DURATION] = 0;
  cmd_profile[index][CMD_NUM_SAMPLES]  = 0;
  cmd_profile[index][CMD_AVG_DURATION] = 0;

  printf("bic-util: perf test on slot%d for ", slot_id);
  if (loopCount)
    printf("%d cycles\n", loopCount);
  else
    printf("infinite cycles\n");

  struct timeval  tv1, tv2;
  while(1) {
    gettimeofday(&tv1, NULL);

    ret = bic_get_dev_id(slot_id, &id, NONE_INTF);
    if (ret) {
      printf("util_perf_test: bic_get_dev_id returns %d, loop=%d\n", ret, i);
      return ret;
    }

    gettimeofday(&tv2, NULL);

    elapsedTime = (((long double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
                   (long double)(tv2.tv_sec - tv1.tv_sec)) * 1000000);

    cmd_profile[index][CMD_AVG_DURATION] += elapsedTime;
    cmd_profile[index][CMD_NUM_SAMPLES] += 1;
    if (cmd_profile[index][CMD_MAX_DURATION] < elapsedTime)
      cmd_profile[index][CMD_MAX_DURATION] = elapsedTime;

    if (cmd_profile[index][CMD_MIN_DURATION] > elapsedTime && elapsedTime > 0)
      cmd_profile[index][CMD_MIN_DURATION] = elapsedTime;

    ++i;

    if ((i & 0xfff) == 0) {
      printf("slot%d stats: loop %d num cmds=%d, avg duration(us)=%d, min duration(us)=%d, max duration(us)=%d\n",
            slot_id, i,
            (int)cmd_profile[index][CMD_NUM_SAMPLES],
            (int)(cmd_profile[index][CMD_AVG_DURATION]/cmd_profile[index][CMD_NUM_SAMPLES]),
            (int)cmd_profile[index][CMD_MIN_DURATION],
            (int)cmd_profile[index][CMD_MAX_DURATION]
          );
    }

    if (loopCount != 0  && i==loopCount) {
      printf("slot%d stats after loop %d\n num cmds=%d, avg duration(us)=%d, min duration(us)=%d, max duration(us)=%d\n",
            slot_id, i,
            (int)cmd_profile[index][CMD_NUM_SAMPLES],
            (int)(cmd_profile[index][CMD_AVG_DURATION]/cmd_profile[index][CMD_NUM_SAMPLES]),
            (int)cmd_profile[index][CMD_MIN_DURATION],
            (int)cmd_profile[index][CMD_MAX_DURATION]
          );
      break;
    }
  }

  return ret;
}

static int
util_read_sensor(uint8_t slot_id) {
  int ret = 0;
  int i = 0;
  int sensor_cnt = 0;
  ipmi_extend_sensor_reading_t sensor = {0};
  uint8_t config_status = 0xff;
  uint8_t *sensor_list;

  ret = bic_is_exp_prsnt(slot_id);
  if ( ret < 0 ) {
    printf("%s() Couldn't get the status of 1OU/2OU\n", __func__);
    return -1;
  }
  config_status = (uint8_t) ret;

  ret = pal_get_fru_sensor_list(slot_id, &sensor_list, &sensor_cnt);
  if (ret < 0) {
    printf("fru%d get sensor list failed!\n", slot_id);
    return ret;
  }

  for (i = 0; i < sensor_cnt; i++) {
    ret = pal_read_bic_sensor(slot_id, sensor_list[i], &sensor, bmc_location, config_status);
    if (ret < 0 ) {
      continue;
    }
    if (sensor.read_type == STANDARD_CMD) {
      printf("sensor num: 0x%02X: value: 0x%02X, flags: 0x%02X, status: 0x%02X, ext_status: 0x%02X\n",
              sensor_list[i], sensor.value, sensor.flags, sensor.status, sensor.ext_status);
    } else {
      printf("sensor num: 0x%02X: value: 0x%04X, flags: 0x%02X\n", sensor_list[i], sensor.value, sensor.flags);
    }
  }

  return ret;
}

static int
util_get_sdr(uint8_t slot_id) {
#define LAST_RECORD_ID 0xFFFF
#define BYTES_ENTIRE_RECORD 0xFF
  int ret = 0;
  uint8_t rlen = 0;
  uint8_t rbuf[MAX_IPMB_RES_LEN] = {0};
  uint8_t intf_list[4] = {NONE_INTF};
  uint8_t intf_index = 0;
  uint8_t config_status = 0xff;
  uint8_t type_2ou = UNKNOWN_BOARD;

  ipmi_sel_sdr_req_t req;
  ipmi_sel_sdr_res_t *res = (ipmi_sel_sdr_res_t *) rbuf;

  ret = bic_is_exp_prsnt(slot_id);
  if ( ret < 0 ) {
    printf("%s() Couldn't get the status of 1OU/2OU\n", __func__);
    return -1;
  }
  config_status = (uint8_t) ret;

  if ( (config_status & PRESENT_1OU) == PRESENT_1OU && (bmc_location != NIC_BMC) ) {
    intf_list[1] = FEXP_BIC_INTF;
  } else if ( (config_status & PRESENT_2OU) == PRESENT_2OU ) {
    if ( fby35_common_get_2ou_board_type(slot_id, &type_2ou) == 0 ) {
      if ( ((type_2ou & DPV2_X8_BOARD) != DPV2_X8_BOARD) &&
           ((type_2ou & DPV2_X16_BOARD) != DPV2_X16_BOARD) ) {
        intf_list[2] = REXP_BIC_INTF;
      }
    } else {
      printf("%s() Failed to get 2OU board type\n", __func__);
    }
  }

  if ( bmc_location == NIC_BMC ) {
    intf_list[3] = BB_BIC_INTF;
  }

  for( intf_index = 0; intf_index < intf_size; intf_index++ ) {
    if ( intf_list[intf_index] == 0 ) {
      continue;
    }

    printf("%s:\n", intf_name[intf_index]);
    req.rsv_id = 0;
    req.rec_id = 0;
    req.offset = 0;
    req.nbytes = BYTES_ENTIRE_RECORD;
    for ( ;req.rec_id != LAST_RECORD_ID; ) {
      ret = bic_get_sdr(slot_id, &req, res, &rlen, intf_list[intf_index]);
      if (ret) {
        printf("util_get_sdr:bic_get_sdr returns %d\n", ret);
        continue;
      }

      sdr_full_t *sdr = (sdr_full_t *)res->data;

      printf("type: %d, ", sdr->type);
      printf("sensor num: 0x%02X, ", sdr->sensor_num);
      printf("type: 0x%02X, ", sdr->sensor_type);
      printf("evt_read_type: 0x%02X, ", sdr->evt_read_type);
      printf("m_val: 0x%02X, ", sdr->m_val);
      printf("m_tol: 0x%02X, ", sdr->m_tolerance);
      printf("b_val: 0x%02X, ", sdr->b_val);
      printf("b_acc: 0x%02X, ", sdr->b_accuracy);
      printf("acc_dir: 0x%02X, ", sdr->accuracy_dir);
      printf("rb_exp: 0x%02X,\n", sdr->rb_exp);

      req.rec_id = res->next_rec_id;
    }
    printf("This record is LAST record\n");
    printf("\n");
  }
  return ret;
}

static int
util_get_postcode(uint8_t slot_id) {
  int ret = 0;
  uint8_t buf[MAX_IPMB_RES_LEN] = {0x0};
  uint8_t len = 0;
  int i = 0;

  ret = bic_get_80port_record(slot_id, buf, &len, NONE_INTF);
  if ( ret < 0 ) {
    printf("%s: Failed to get POST code of slot%d\n", __func__, slot_id);
    return ret;
  }

  printf("%s: returns %d bytes\n", __func__, len);
  for (i = 0; i < len; i++) {
    if (!(i % 16) && i)
      printf("\n");

    printf("%02X ", buf[i]);
  }
  printf("\n");
  return ret;
}

static int
util_print_dword_postcode_buf(uint8_t slot_id) {
  int ret = 0;
  size_t len;
  size_t intput_len = 0;
  size_t i;
  uint32_t * dw_postcode_buf = malloc( MAX_POSTCODE_NUM * sizeof(uint32_t));
  if (dw_postcode_buf) {
    intput_len = MAX_POSTCODE_NUM;
  } else {
    syslog(LOG_ERR, "%s Error, failed to allocate dw_postcode buffer", __func__);
    intput_len = 0;
    return -1;
  }

  ret = bic_request_post_buffer_dword_data(slot_id, dw_postcode_buf, intput_len, &len);
  if (ret) {
    printf("bic_request_post_buffer_dword_data: returns %d\n", ret);
    free(dw_postcode_buf);
    return ret;
  }
  printf("util_get_post_buf: returns %zu dword\n", len);
  for (i = 0; i < len; i++) {
    if (!(i % 4) && i)
      printf("\n");

    printf("[%08X] ", dw_postcode_buf[i]);
  }
  printf("\n");
  if(dw_postcode_buf)
    free(dw_postcode_buf);

  return ret;

}

static int
util_bic_clear_cmos(uint8_t slot_id) {
  return pal_clear_cmos(slot_id);
}

int
util_check_usb_port(uint8_t slot_id, char *comp) {
  uint8_t tbuf[512], rbuf[512] = {0};
  uint8_t fw_comp;
  int i, ret = -1;
  int transferlen = sizeof(tbuf);
  int transferred = 0, received = 0;
  int retries = 3;
  usb_dev bic_udev;
  usb_dev *udev = &bic_udev;

  if (comp == NULL) {
    syslog(LOG_ERR, "%s: comp shouldn't be null!", __func__);
    return -1;
  }

  for (i = 0; i < transferlen; ++i) {
    tbuf[i] = i + 1;
  }

  udev->handle = NULL;
  udev->ci = 1;
  udev->epaddr = USB_INPUT_PORT;

  if (strcmp("sb", comp) == 0) {
    fw_comp = FW_BIOS;
  } else if (strcmp("1ou", comp) == 0) {
    fw_comp = FW_1OU_CXL;
  } else {
    fw_comp = 0xFF;
  }

  // init usb device
  ret = bic_init_usb_dev(slot_id, fw_comp, udev, SB_USB_PRODUCT_ID, SB_USB_VENDOR_ID);
  if (ret < 0) {
    goto exit;
  }

  printf("Input test data, USB timeout: 3000ms\n");
  while (true) {
    ret = libusb_bulk_transfer(udev->handle, udev->epaddr, tbuf, transferlen, &transferred, 3000);
    if ((ret != 0) || (transferlen != transferred)) {
      printf("Error in transferring data! err = %d (%s) and transferred = %d (expected length %d)\n",
             ret, libusb_error_name(ret), transferred, transferlen);
      if (!(--retries)) {
        ret = -1;
        break;
      }
      msleep(100);
    } else {
      break;
    }
  }
  if (ret != 0) {
    goto exit;
  }

  udev->epaddr = USB_OUTPUT_PORT;
  retries = 3;
  while (true) {
    ret = libusb_bulk_transfer(udev->handle, udev->epaddr, rbuf, sizeof(rbuf), &received, 3000);
    if (ret != 0) {
      printf("Error in receiving data! err = %d (%s)\n", ret, libusb_error_name(ret));
      if (!(--retries)) {
        ret = -1;
        break;
      }
    } else {
      //printf("Received length: %d\n", received);
      printf("Received data: ");
      for (i = 0; i < received; ++i) {
        printf("%02x ", rbuf[i]);
      }
      printf("\n");
      break;
    }
  }

exit:
  if (ret != 0) {
    printf("Check USB port Failed\n");
  } else {
    printf("Check USB port Successful\n");
  }
  printf("\n");

  // close usb device
  bic_close_usb_dev(udev);

  return ret;
}

int
main(int argc, char **argv) {
  uint8_t slot_id = 0;
  uint8_t is_fru_present = 0;
  int ret = 0;
  uint8_t gpio_num = 0;
  uint8_t gpio_val = 0;
  int i = 0;

  if (argc < 3) {
    goto err_exit;
  }

  ret = fby35_common_get_slot_id(argv[1], &slot_id);
  if ( ret < 0 ) {
    printf("%s is invalid!\n", argv[1]);
    goto err_exit;
  }

  ret = fby35_common_get_bmc_location(&bmc_location);
  if ( ret < 0 ) {
    printf("%s() Couldn't get the location of BMC\n", __func__);
    return -1;
  }

  if ( slot_id != FRU_ALL ) {
    if ( bmc_location == NIC_BMC && slot_id != FRU_SLOT1 ) {
      printf("slot%d is not supported!\n", slot_id);
      return -1;
    }

    ret = pal_is_fru_prsnt(slot_id, &is_fru_present);
    if ( ret < 0 || is_fru_present == 0 ) {
      printf("%s is not present!\n", argv[1]);
      return -1;
    }
  }

  if ( strncmp(argv[2], "--", 2) == 0 ) {
    if ( strcmp(argv[2], "--get_gpio") == 0 ) {
      if ( argc != 3 ) {
        goto err_exit;
      }
      return util_get_gpio(slot_id);
    } else if ( strcmp(argv[2], "--set_gpio") == 0 ) {
      if ( argc != 5 ) {
        goto err_exit;
      }

      gpio_num = atoi(argv[3]);
      gpio_val = atoi(argv[4]);
      if ( gpio_val > 1 ) goto err_exit;
      return util_set_gpio(slot_id, gpio_num, gpio_val);
    } else if ( strcmp(argv[2], "--check_status") == 0 ) {
      if ( argc != 3 ) {
        goto err_exit;
      }
      return util_check_status(slot_id);
    } else if ( strcmp(argv[2], "--get_dev_id") == 0 ) {
      if ( argc != 3 ) {
        goto err_exit;
      }
      return util_get_device_id(slot_id);
    } else if ( strcmp(argv[2], "--get_sdr") == 0 ) {
      if ( argc != 3 ) {
        goto err_exit;
      }
      return util_get_sdr(slot_id);
    } else if ( strcmp(argv[2], "--read_sensor") == 0 ) {
      if ( argc != 3 ) {
        goto err_exit;
      }
      return util_read_sensor(slot_id);
    } else if ( strcmp(argv[2], "--reset") == 0 ) {
      if ( argc != 3 ) {
        goto err_exit;
      }
      return util_bic_reset(slot_id);
    } else if ( strcmp(argv[2], "--get_post_code") == 0 ) {
      if ( argc != 3 ) {
        goto err_exit;
      }
      if (fby35_common_get_slot_type(slot_id) == SERVER_TYPE_HD) {
        return util_print_dword_postcode_buf(slot_id);
      } else {
        return util_get_postcode(slot_id);
      }
    } else if ( strcmp(argv[2], "--perf_test") == 0 ) {
      if ( argc != 4 ) {
        goto err_exit;
      }
      else return util_perf_test(slot_id, atoi(argv[3]));
    } else if (!strcmp(argv[2], "--clear_cmos")) {
      if ( argc != 3 ) {
        goto err_exit;
      }
      return util_bic_clear_cmos(slot_id);
    } else if ( strcmp(argv[2], "--file") == 0 ) {
      if ( argc != 4 ) {
        goto err_exit;
      }
      if ( slot_id == FRU_ALL ) {
        if ( bmc_location != NIC_BMC ) {
          for ( i = FRU_SLOT1; i <= FRU_SLOT4; i++ ) {
            ret = pal_is_fru_prsnt(i, &is_fru_present);
            if ( ret < 0 || is_fru_present == 0 ) {
              printf("slot%d is not present!\n", i);
            } else process_file(i, argv[3]);
          }
        } else process_file(FRU_SLOT1, argv[3]);
        return 0;
      } else return process_file(slot_id, argv[3]);
    } else if ( strcmp(argv[2], "--check_usb_port") == 0 ) {
      if ( argc != 4 ) {
        goto err_exit;
      }
      if ( (strcmp("sb", argv[3]) != 0) && (strcmp("1ou", argv[3]) != 0) && (strcmp("2ou", argv[3]) != 0) ) {
        printf("Invalid component: %s\n", argv[3]);
        goto err_exit;
      }
      return util_check_usb_port(slot_id, argv[3]);
    } else {
      printf("Invalid option: %s\n", argv[2]);
      goto err_exit;
    }
  } else if (argc >= 4) {
    if (util_is_numeric(argv + 2)) {
      return process_command(slot_id, (argc - 2), (argv + 2));
    } else {
      goto err_exit;
    }
  } else {
    goto err_exit;
  }

err_exit:
  print_usage_help();
  return -1;
}
