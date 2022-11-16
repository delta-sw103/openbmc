#ifndef __SYSFPGA_I2C_H__
#define __SYSFPGA_I2C_H__

#include <linux/i2c.h>
#include "sysfpga.h"

/* Bus number */
enum {
    I2C_PLD_CH = 0,
};

/*
* n is bus number
* b is base register offset = SYSFPGA_I2C_BASE_REG(n)
*/

/* Base register offset */
//#define SYSFPGA_1588_DPLL_BASE_REG   0x300
//#define SYSFPGA_1588_FPGA_BASE_REG   0x600
#define SYSFPGA_I2C_BASE_REG(n)     (0x1200 + (0x300 * n))

/* Register (Only for PLD) */
#define GRAB_ENABLE(b)              (b)
#define GRAB_FAIL_STATUS_0(b)       (b + 0x4)
#define GRAB_FAIL_STATUS_1(b)       (b + 0x8)
#define GRAB_CTRL(b)                (b + 0x10)
#define GRAB_ADDR(b)                (b + 0x14)
#define GRAB_DATA(b)                (b + 0x18)

/**** Bit offset ****/
// Enable register
#define GRABBER_READY_MASK                  BIT(16)
#define GRABBER_ENABLE_MASK                 BIT(0)
// Fail status register 1
#define GRABBER_FAIL_MASK                   BIT(16)
// Control register
#define CTRL_DEVICE_ADDR_OFFSET             25
#define CTRL_DATA_LENGTH_OFFSET             15
#define CTRL_REGADDR_LENGTH_OFFSET          8
#define CTRL_RW_MASK                        BIT(3)
// Address register
#define ADDR_954X_DEVICE_ADDR_MASK(a)       (a << 25)
#define ADDR_UPPER_REGADDR_MASK(a)          (a << 8)
#define ADDR_LOWER_REGADDR_MASK(a)          (a)

/* timeout */
#define SYSFPGA_I2C_TIMEOUT                 100000  /* 100000us = 100ms */

/* data length */
#define SYSFPGA_I2C_MAX_DATA_LENGTH         8

struct sysfpga_i2c_s {
    char name[48];
    int bus;
    int offset;
};

int delta_sysfpga_i2c_adapter_init(struct i2c_client *client, struct fpga_i2c_bus_s *fpga_data);
int delta_sysfpga_i2c_adapter_exit(struct fpga_i2c_bus_s *fpga_data);

#endif /* __SYSFPGA_I2C_H__ */