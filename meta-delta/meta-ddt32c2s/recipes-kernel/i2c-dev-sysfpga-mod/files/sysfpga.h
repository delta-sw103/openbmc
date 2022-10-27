#ifndef __SYSFPGA_H__
#define __SYSFPGA_H__

#include <linux/i2c.h>
#include <i2c_dev_sysfs.h>

/* For fpga_protect_write */
#define FPGA_WRITE_CTRL	0x10          // FPGA write control register
#define BMC_WRITE_CTRL	0x00000100    // FPGA write control mask
#define CPU_WRITE_CTRL	0x00000010
#define WRITE_CTRL_SEL	0x00000001

struct sysfpga_data_s {
    i2c_dev_data_st *i2c_dev_data;
    struct i2c_adapter adap;
};

#endif /* __SYSFPGA_H__ */