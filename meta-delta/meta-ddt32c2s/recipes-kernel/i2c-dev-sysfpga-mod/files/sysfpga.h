#ifndef __SYSFPGA_H__
#define __SYSFPGA_H__

#include <linux/i2c.h>
#include <i2c_dev_sysfs.h>

/* For fpga_protect_write */
#define FPGA_WRITE_CTRL	0x10          // FPGA write control register
#define BMC_WRITE_CTRL	0x00000100    // FPGA write control mask
#define CPU_WRITE_CTRL	0x00000010
#define WRITE_CTRL_SEL	0x00000001

struct fpga_i2c_bus_s {
    struct i2c_client *client;
    struct i2c_adapter *adap;
    int offset;
};

ssize_t i2c_fpga_read(struct device *dev, const int reg, uint32_t *reg_val);
ssize_t i2c_fpga_write(struct device *dev, const int reg, uint32_t result);

#endif /* __SYSFPGA_H__ */