/*
 * delta_sysfpga.c - The i2c driver for DD-T32C2S SYSFPGA
 *
 * Copyright 2022-present Delta Eletronics, Inc. All Rights Reserved.
 *
 */

//#define DEBUG

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <i2c_dev_sysfs.h>

#ifdef DEBUG

#define SYSFPGA_DEBUG(fmt, ...) do {                   \
  printk(KERN_DEBUG "%s:%d " fmt "\n",            \
         __FUNCTION__, __LINE__, ##__VA_ARGS__);  \
} while (0)

#else /* !DEBUG */

#define SYSFPGA_DEBUG(fmt, ...)

#endif

static ssize_t fpga_write(struct device *dev,
                          struct device_attribute *attr,
                          const char *buf, size_t count)
{
  struct i2c_client *client = to_i2c_client(dev);
  i2c_dev_data_st *data = i2c_get_clientdata(client);
  i2c_sysfs_attr_st *i2c_attr = TO_I2C_SYSFS_ATTR(attr);
  const i2c_dev_attr_st *dev_attr = i2c_attr->isa_i2c_attr;
  int i, mask, ret;
  uint32_t val, result = 0;
  uint8_t values[4];

  if (sscanf(buf, "%i", &val) <= 0) {
    SYSFPGA_DEBUG("val\n");
    return -EINVAL;
  }

  result |= val << dev_attr->ida_bit_offset;

  values[0] = (result >> 24) & 0xff;
  values[1] = (result >> 16) & 0xff;
  values[2] = (result >> 8) & 0xff;
  values[3] = result & 0xff;

  mutex_lock(&data->idd_lock);
  
  for (i = 0; i < 4; i++) {
    ret = i2c_smbus_write_byte_data(client, dev_attr->ida_reg + i, values[i]);
    if (ret < 0) {
      /* error case */
      mutex_unlock(&data->idd_lock);
      SYSFPGA_DEBUG("I2C write error, ret: %d, byte cnt: %d\n", ret, i);
      return ret;
    }
  }
  
  mutex_unlock(&data->idd_lock);

  return count;
}

static ssize_t fpga_read(struct device *dev,
                         struct device_attribute *attr,
                         char *buf)
{
  struct i2c_client *client = to_i2c_client(dev);
  i2c_dev_data_st *data = i2c_get_clientdata(client);
  i2c_sysfs_attr_st *i2c_attr = TO_I2C_SYSFS_ATTR(attr);
  const i2c_dev_attr_st *dev_attr = i2c_attr->isa_i2c_attr;
  int result;
  int mask, ret;
  uint8_t values[4] = {};
  uint32_t reg_val = 0;

  ret = i2c_dev_read_nbytes(dev, attr, values, 4);
  if (ret < 0) {
    /* error case */
    return ret;
  }
  //SYSFPGA_DEBUG("dev_attr->ida_reg %#x", dev_attr->ida_reg);
  //SYSFPGA_DEBUG("valus[0]-[3] %#x, %#x, %#x, %#x\n", values[0], values[1], values[2], values[3]);

  mask = (1 << (dev_attr->ida_n_bits)) - 1;
  reg_val = (values[3] << 24) | (values[2] << 16) |
           (values[1] << 8) | values[0];
  result = (reg_val >> dev_attr->ida_bit_offset) & mask;

  return scnprintf(buf, PAGE_SIZE,
                   "%#x\n\nNote:\n%s\n"
                   "Bit[%d:%d] @ register %#X, register value 0x%08X\n",
                   result, (dev_attr->ida_help) ? dev_attr->ida_help : "",
                   dev_attr->ida_bit_offset + dev_attr->ida_n_bits - 1,
                   dev_attr->ida_bit_offset, dev_attr->ida_reg, reg_val);
}

#define present_help_str                          \
  "0: Present\n"                                  \
  "1: Not Present\n"


static const i2c_dev_attr_st sysfpga_attr_table[] = {
    {
        "sysfpga_ver",
        "Code revision controlled by code editor\n",
        fpga_read,
        NULL,
        0x04, 0, 8,
    },
    {
        "psu1_present",
        "PSU1: \n" present_help_str,
        fpga_read,
        NULL,
        0x20, 16, 1,
    },
    {
        "psu2_present",
        "PSU2: \n" present_help_str,
        fpga_read,
        NULL,
        0x20, 20, 1,
    },
    {
        "test_reg",
        "Test register for BMC, read/Write in little endian.\n",
        fpga_read,
        fpga_write,
        0xF0, 0, 32,
    },
};

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
static int sysfpga_detect(struct i2c_client* client,
                          struct i2c_board_info* info)
{
  /*
   * We don't currently do any detection of SYSFPGA
   */
  strlcpy(info->type, "sysfpga", I2C_NAME_SIZE);
  return 0;
}

static int sysfpga_probe(struct i2c_client* client,
                         const struct i2c_device_id* id)
{
  int n_attrs = sizeof(sysfpga_attr_table) / sizeof(sysfpga_attr_table[0]);
  struct device *dev = &client->dev;
  i2c_dev_data_st *sysfpga_data;

  sysfpga_data = devm_kzalloc(dev, sizeof(i2c_dev_data_st), GFP_KERNEL);
  if (!sysfpga_data) {
    return -ENOMEM;
  }

  return i2c_dev_sysfs_data_init(
      client, sysfpga_data, sysfpga_attr_table, n_attrs);
}

static int sysfpga_remove(struct i2c_client* client) {
  i2c_dev_data_st *sysfpga_data = i2c_get_clientdata(client);
  i2c_dev_sysfs_data_clean(client, sysfpga_data);
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
