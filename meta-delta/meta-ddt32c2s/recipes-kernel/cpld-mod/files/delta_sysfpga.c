/*
 * delta_sysfpga.c - The i2c driver for DD-T32C2S SYSFPGA
 *
 * Copyright 2022-present Delta Eletronics, Inc. All Rights Reserved.
 *
 */
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <i2c_dev_sysfs.h>

#ifdef DEBUG

#define PP_DEBUG(fmt, ...) do {                   \
  printk(KERN_DEBUG "%s:%d " fmt "\n",            \
         __FUNCTION__, __LINE__, ##__VA_ARGS__);  \
} while (0)

#else /* !DEBUG */

#define PP_DEBUG(fmt, ...)

#endif

#define present_help_str                          \
  "0: Present\n"                                  \
  "1: Not Present\n"

static const i2c_dev_attr_st sysfpga_attr_table[] = {
    {
        "psu1_present",
        "PSU1: \n" present_help_str,
        I2C_DEV_ATTR_SHOW_DEFAULT,
        NULL,
        0x20,
        16,
        1,
    },
    {
        "psu2_present",
        "PSU2: \n" present_help_str,
        I2C_DEV_ATTR_SHOW_DEFAULT,
        NULL,
        0x20,
        20,
        1,
    },
};

static i2c_dev_data_st sysfpga_data;

/*
 * SYSFPGA i2c addresses.
 */
static const unsigned short normal_i2c[] = {0x36, I2C_CLIENT_END};

/* SYSFPGA id */
static const struct i2c_device_id sysfpga_id[] = {
    {"sysfpga", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, sysfpga_id);

/* Return 0 if detection is successful, -ENODEV otherwise */
static int sysfpga_detect(
    struct i2c_client* client,
    struct i2c_board_info* info) {
  /*
   * We don't currently do any detection of SYSFPGA
   */
  strlcpy(info->type, "sysfpga", I2C_NAME_SIZE);
  return 0;
}

static int sysfpga_probe(
    struct i2c_client* client,
    const struct i2c_device_id* id) {
  int n_attrs = sizeof(sysfpga_attr_table) / sizeof(sysfpga_attr_table[0]);
  return i2c_dev_sysfs_data_init(
      client, &sysfpga_data, sysfpga_attr_table, n_attrs);
}

static int sysfpga_remove(struct i2c_client* client) {
  i2c_dev_sysfs_data_clean(client, &sysfpga_data);
  return 0;
}

static struct i2c_driver sysfpga_driver = {
    .class = I2C_CLASS_HWMON,
    .driver =
        {
            .name = "sysfpga",
        },
    .probe = sysfpga_probe,
    .remove = sysfpga_remove,
    .id_table = sysfpga_id,
    .detect = sysfpga_detect,
    .address_list = normal_i2c,
};

static int __init sysfpga_mod_init(void) {
  return i2c_add_driver(&sysfpga_driver);
}

static void __exit sysfpga_mod_exit(void) {
  i2c_del_driver(&sysfpga_driver);
}

MODULE_AUTHOR("Samantha Chu");
MODULE_DESCRIPTION("SYSFPGA Driver");
MODULE_LICENSE("GPL");

module_init(sysfpga_mod_init);
module_exit(sysfpga_mod_exit);
