#ifndef __SYSFPGA_I2C_H__
#define __SYSFPGA_I2C_H__

#include <linux/i2c.h>
#include "sysfpga.h"

/* Register offset */
#define SYSFPGA_1588_DPLL_I2C_BASE_OFFSET   0x300
#define SYSFPGA_1588_FPGA_I2C_BASE_OFFSET   0x600
#define SYSFPGA_I2C_BASE_OFFSET(n)          (0x1000 + (0x300 * n))

enum {
    I2C_PLD_CH = 0,
    I2C_MON_CH,
    I2C_PORT_CH0,
    I2C_PORT_CH1,
    I2C_PORT_CH2,
    I2C_PORT_CH3,
};

#define CTRL_REG(n) (n + 0x4)
#define ADDR_REG(n) (n + 0x8)
#define DATA_REG(n) (n + 0x100)

/* Register bit offset */
/* Control register */
#define SYSFPGA_I2C_DEVICE_ADDR_OFFSET      25
#define SYSFPGA_I2C_DATA_LENGTH_OFFSET      15
#define SYSFPGA_I2C_954X_ENABLE_OFFSET      13
#define SYSFPGA_I2C_954X_CH_SEL_OFFSET      10
#define SYSFPGA_I2C_REGADDR_LENGTH_OFFSET   8
#define SYSFPGA_I2C_RW_OFFSET               3
#define SYSFPGA_I2C_READY_MASK              0x4
#define SYSFPGA_I2C_FAIL_STATUS_MASK        0x2
#define SYSFPGA_I2C_START_OFFSET            0
/* Address register */
#define SYSFPGA_I2C_954X_DEVICE_ADDR        25
#define SYSFPGA_I2C_UPPER_REGADDR           8
#define SYSFPGA_I2C_LOWER_REGADDR           0

/* timeout */
#define SYSFPGA_I2C_TIMEOUT                 100000  /* 100000us = 100ms */

/* data length */
#define SYSFPGA_I2C_MAX_DATA_LENGTH         256

struct sysfpga_i2c_s {
    char name[48];
    int bus;
    int offset;
};

int delta_sysfpga_i2c_adapter_init(struct i2c_client *client, struct fpga_i2c_bus_s *fpga_data);
int delta_sysfpga_i2c_adapter_exit(struct fpga_i2c_bus_s *fpga_data);

#endif /* __SYSFPGA_I2C_H__ */