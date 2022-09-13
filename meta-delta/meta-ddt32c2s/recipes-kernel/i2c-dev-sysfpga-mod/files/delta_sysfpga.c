/*
 * delta_sysfpga.c - The i2c driver for DD-T32C2S SYSFPGA
 *
 * Copyright 2022-present Delta Eletronics, Inc. All Rights Reserved.
 *
 */

#define DEBUG

#include <linux/delay.h>
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

/* For fpga_protect_write */
#define FPGA_WRITE_CTRL	0x10          // FPGA write control register
#define BMC_WRITE_CTRL	0x00000100    // FPGA write control mask
#define CPU_WRITE_CTRL	0x00000010
#define WRITE_CTRL_SEL	0x00000001

static ssize_t i2c_fpga_read(struct device *dev,
                             const int reg, uint32_t *reg_val)
{
  struct i2c_client *client = to_i2c_client(dev);
  i2c_dev_data_st *data = i2c_get_clientdata(client);
  struct i2c_msg msg[1];
  char xfer[4] = {};
  uint8_t values[4] = {};
  int ret = 0, i = 0;

  memset(msg, 0, sizeof(msg));
  
  /* Set the register address */
  if (reg > 0xff) {   // For 2 byte register address
    xfer[1] = (reg >> 8) & 0xFF;
  }
  xfer[0]       = reg & 0xFF;
  msg[0].addr   = client->addr;
  msg[0].buf    = xfer;
  msg[0].len    = 4;

  mutex_lock(&data->idd_lock);
  ret = i2c_transfer(client->adapter, msg, 1);
  mutex_unlock(&data->idd_lock);
  if (ret < 0) {
    SYSFPGA_DEBUG("Set the register %#x failed!\n", reg);
    return ret;
  }

  /* Read data */
  mutex_lock(&data->idd_lock);
  for (i = 0; i < 4; ++i) {
    values[i] = i2c_smbus_read_byte_data(client, reg + i);
  }
  mutex_unlock(&data->idd_lock);

  *reg_val = (values[3] << 24) | (values[2] << 16) |
             (values[1] << 8) | values[0];

  return ret;
}

static ssize_t i2c_fpga_write(struct device *dev,
                             const int reg, uint32_t result)
{
  struct i2c_client *client = to_i2c_client(dev);
  i2c_dev_data_st *data = i2c_get_clientdata(client);
  struct i2c_msg msg[1];
  char xfer[8] = {};
  int ret = 0;

  memset(msg, 0, sizeof(msg));
  
  /* Set the register address */
  if (reg > 0xff) {   // For 2 byte register address
    xfer[1] = (reg >> 8) & 0xFF;
  }
  xfer[0]       = reg & 0xFF;
  /* Write back the new result in LSB */
  xfer[4] = result & 0xff;
  xfer[5] = (result >> 8) & 0xff;
  xfer[6] = (result >> 16) & 0xff;
  xfer[7] = (result >> 24) & 0xff;
  msg[0].addr   = client->addr;
  msg[0].buf    = xfer;
  msg[0].len    = 8;

  mutex_lock(&data->idd_lock);
  ret = i2c_transfer(client->adapter, msg, 1);
  mutex_unlock(&data->idd_lock);
  if (ret < 0) {
    SYSFPGA_DEBUG("Write data to %#x failed!", reg);
    return ret;
  }

  return ret;
}

static ssize_t fpga_read(struct device *dev,
                         struct device_attribute *attr,
                         char *buf)
{
  i2c_sysfs_attr_st *i2c_attr = TO_I2C_SYSFS_ATTR(attr);
  const i2c_dev_attr_st *dev_attr = i2c_attr->isa_i2c_attr;
  int mask, ret;
  uint32_t reg_val = 0, result = 0;

  ret = i2c_fpga_read(dev, dev_attr->ida_reg, &reg_val);

  /* Reorganize the data and extract the needed one */
  mask = (1 << (dev_attr->ida_n_bits)) - 1;
  result = (reg_val >> dev_attr->ida_bit_offset) & mask;

  return scnprintf(buf, PAGE_SIZE,
                   "%#x\n\nNote:\n%s\n"
                   "Bit[%d:%d] @ register %#X, register value 0x%08X\n",
                   result, (dev_attr->ida_help) ? dev_attr->ida_help : "",
                   dev_attr->ida_bit_offset + dev_attr->ida_n_bits - 1,
                   dev_attr->ida_bit_offset, dev_attr->ida_reg, reg_val);
}

static ssize_t fpga_write(struct device *dev,
                          struct device_attribute *attr,
                          const char *buf, size_t count)
{
  i2c_sysfs_attr_st *i2c_attr = TO_I2C_SYSFS_ATTR(attr);
  const i2c_dev_attr_st *dev_attr = i2c_attr->isa_i2c_attr;
  int mask, ret;
  uint32_t val = 0, result = 0;

  /* Read the original data and store in result */
  ret = i2c_fpga_read(dev, dev_attr->ida_reg, &result);

  /* Get the write data from buf */
  if (sscanf(buf, "%i", &val) <= 0) {
    SYSFPGA_DEBUG("val\n");
    return -EINVAL;
  }

  /* Make sure the write data doesn't exceed */
  mask = (1 << (dev_attr->ida_n_bits)) - 1;
  val &= mask;

  /* Clear the bits and replace the original data with new value */
  result &= ~(mask << dev_attr->ida_bit_offset);
  result |= val << dev_attr->ida_bit_offset;

  /* Write back the new result */
  ret = i2c_fpga_write(dev, dev_attr->ida_reg, result);

  return count;
}

static ssize_t fpga_protect_write(struct device *dev,
                          struct device_attribute *attr,
                          const char *buf, size_t count)
{
  int ret, waiting = 0;
  uint32_t token = 0;

  /* Check write control */
  /* Read the write_control register data */
  ret = i2c_fpga_read(dev, FPGA_WRITE_CTRL, &token);

  while (token & CPU_WRITE_CTRL) {
    /* CPU owns the write control, wait for 2 seconds */
		if ( waiting > 5 ) {
			/* Have waited for 2 seconds */
			SYSFPGA_DEBUG("CPU owns the write control, get BMC write control failed.");
			SYSFPGA_DEBUG("Note: You need to wait until CPU release the write control.");
			return EFAULT;
		}
		msleep(400);
		waiting++;
		ret = i2c_fpga_read(dev, FPGA_WRITE_CTRL, &token);
  }

  /* Get write control */
  ret = i2c_fpga_write(dev, FPGA_WRITE_CTRL, BMC_WRITE_CTRL);
  /* Check owner is BMC */
	ret = i2c_fpga_read(dev, FPGA_WRITE_CTRL, &token);

  if (!(token & WRITE_CTRL_SEL)) {
		SYSFPGA_DEBUG("Error: Get BMC write control failed, still owned by CPU.");
		return EFAULT;
	} else if (!(token & BMC_WRITE_CTRL)) {
		SYSFPGA_DEBUG("Error: Didn't set BMC write control.");
		return EFAULT;
	}

  /* Normal write */
  ret = fpga_write(dev, attr, buf, count);

  /* Release write control */
  ret = i2c_fpga_write(dev, FPGA_WRITE_CTRL, 0);

  return count;
}


#define present_help_str                          \
  "0: Present\n"                                  \
  "1: Not Present\n"


static const i2c_dev_attr_st sysfpga_attr_table[] = {
    {
        "fpga_ver",
        NULL,
        fpga_read,
        NULL,
        0x04, 0, 8,
    },
    {
        "image_boot_block",
        "0/3: None\n"
        "1: Golden Image\n"
        "2: Application Image\n",
        fpga_read,
        NULL,
        0x04, 14, 2,
    },
    {
        "code_revision_date",
        "[31:24]Code Released Year\n"
        "[23:16]Code Released Month\n"
        "[15: 8]Code Released Day\n"
        "[ 7: 0]Code revision for RD test purpose\n",
        fpga_read,
        NULL,
        0x08, 0, 32,
    },
    {
        "write_ctrl",
        "0: Write particular registers by CPU\n"
        "1: Write particular registers by BMC\n",
        fpga_read,
        NULL,
        0x10, 0, 1,
    },
    {
        "cpu_req_write_ctrl",
        "CPU request the write control\n"
        "0: CPU discard\n"
        "1: CPU request\n",
        fpga_read,
        NULL,
        0x10, 4, 1,
    },
    {
        "bmc_req_write_ctrl",
        "BMC request the write control\n"
        "0: BMC discard\n"
        "1: BMC request\n",
        fpga_read,
        fpga_write,
        0x10, 8, 1,
    },
    {
        "bmc_sku",
        "0: BMC absent (nonBMC)\n"
        "1: BMC present\n",
        fpga_read,
        NULL,
        0x10, 30, 1,
    },
    {
        "bmc_ready",
        "0: BMC isn't ready\n"
        "1: BMC is ready\n",
        fpga_read,
        NULL,
        0x10, 31, 1,
    },
    {
        "psu1_enable",
        "0: Disable\n"
        "1: Enable(Default)\n",
        fpga_read,
        fpga_write,
        0x1C, 0, 1,
    },
    {
        "psu2_enable",
        "0: Disable\n"
        "1: Enable(Default)\n",
        fpga_read,
        fpga_write,
        0x1C, 1, 1,
    },
    {
        "mb_pwr_enable",
        "0: Disable\n"
        "1: Enable(Default)\n",
        fpga_read,
        fpga_write,
        0x1C, 4, 1,
    },
    {
        "cpu_disable",
        "0: None(Default)\n"
        "1: Disable power\n",
        fpga_read,
        fpga_write,
        0x1C, 8, 1,
    },
    {
        "psu1_present",
        present_help_str,
        fpga_read,
        NULL,
        0x20, 16, 1,
    },
    {
        "psu2_present",
        present_help_str,
        fpga_read,
        NULL,
        0x20, 20, 1,
    },
    {
        "system_pwr",
        "0: System off\n"
        "1: System on\n",
        fpga_read,
        fpga_protect_write,
        0x30, 8, 1,
    },
    {
        "test_reg",
        "Test register for BMC, read/Write in MSB.\n",
        fpga_read,
        fpga_write,
        0xF0, 0, 32,
    },
    {
        "jtag_mux_ctrl",
        "Control JTAG MUX.\n",
        fpga_read,
        fpga_protect_write,
        0x100, 0, 10,
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
  SYSFPGA_DEBUG("client->name %s", &client->name);
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
