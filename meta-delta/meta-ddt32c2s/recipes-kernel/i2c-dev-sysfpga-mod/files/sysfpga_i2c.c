/*
 * sysfpga_i2c.c - Create i2c adapter for DD-T32C2S SYSFPGA i2c master
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
#include "sysfpga.h"
#include "sysfpga_i2c.h"

#ifdef DEBUG

#define SYSFPGA_DEBUG(fmt, ...) do {                   \
  printk(KERN_DEBUG "%s:%d " fmt "\n",            \
         __FUNCTION__, __LINE__, ##__VA_ARGS__);  \
} while (0)

#else /* !DEBUG */

#define SYSFPGA_DEBUG(fmt, ...)

#endif

struct mutex fpga_lock;

/* All sysfpga i2c masters
struct sysfpga_i2c_s sysfpga_i2c_info[] = {
    {"FPGA I2CBUS - PLD"         , 0, SYSFPGA_I2C_BASE_OFFSET(I2C_PLD_CH)},
    {"FPGA I2CBUS - MON"         , 1, SYSFPGA_I2C_BASE_OFFSET(I2C_MON_CH)},
    {"FPGA I2CBUS - PORT_0"      , 2, SYSFPGA_I2C_BASE_OFFSET(I2C_PORT_CH0)},
    {"FPGA I2CBUS - PORT_1"      , 3, SYSFPGA_I2C_BASE_OFFSET(I2C_PORT_CH1)},
    {"FPGA I2CBUS - PORT_2"      , 4, SYSFPGA_I2C_BASE_OFFSET(I2C_PORT_CH2)},
    {"FPGA I2CBUS - PORT_3"      , 5, SYSFPGA_I2C_BASE_OFFSET(I2C_PORT_CH3)},
    {"FPGA I2CBUS - 1588 DPLL"   , 6, SYSFPGA_1588_DPLL_I2C_BASE_OFFSET},
    {"FPGA I2CBUS - 1588 FPGA"   , 7, SYSFPGA_1588_FPGA_I2C_BASE_OFFSET},
};*/

/* Only Port_PLD */
static const struct sysfpga_i2c_s sysfpga_i2c_info[] = {
    {"FPGA I2CBUS - PLD", I2C_PLD_CH, SYSFPGA_I2C_BASE_OFFSET(I2C_PLD_CH)},
};

static int delta_sysfpga_i2c_read(struct i2c_adapter *adap, struct i2c_msg *msgs)
{
    struct fpga_i2c_bus_s *adap_data = i2c_get_adapdata(adap);
    uint32_t ctrl_data = 0, addr_data = 0, rw_data = 0;
    int reg_length = 1, rw = 0;
    int ret = 0;

    if (msgs->addr > 0xff)
        reg_length = 2;

    //set address
    addr_data = msgs->buf[0];
    ret = i2c_fpga_write(adap->dev.parent, ADDR_REG(adap_data->offset), addr_data);
    //set control
    ctrl_data |= ( msgs->addr & 0x7f ) << SYSFPGA_I2C_DEVICE_ADDR_OFFSET;
    ctrl_data |= ( msgs->len & 0x1ff ) << SYSFPGA_I2C_RED_LENGTH_OFFSET;
    ctrl_data |= ( reg_length & 0x3 ) << SYSFPGA_I2C_REGADDR_LENGTH_OFFSET;
    ctrl_data |= ( rw & 0x1 ) << SYSFPGA_I2C_RW_OFFSET;
    ctrl_data |= 0x1 << SYSFPGA_I2C_START_OFFSET;
    SYSFPGA_DEBUG("\tctrl_data = 0x%8X", ctrl_data);
    ret = i2c_fpga_write(adap->dev.parent, CTRL_REG(adap_data->offset), ctrl_data);
    //read
    //get data

    return 0;
}
/*
static int delta_sysfpga_i2c_write()
*/
static int delta_sysfpga_i2c_master_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    int four_byte_num = 0, byte_num = 0;
    int i = 0, ret = 0, waiting = 0;
    uint32_t token = 0;

    //mutex_lock(&fpga_lock);

    
    SYSFPGA_DEBUG("    msgs->addr = %#X\t\t msgs->flags = %#X", msgs->addr, msgs->flags);
    SYSFPGA_DEBUG("    msgs->len  = %d\t\t num = %d", msgs->len, num);
    for (i=0; i<msgs->len; i++)
        SYSFPGA_DEBUG("    msgs->buf = %#X", msgs->buf[i]);
    
    mutex_unlock(&fpga_lock);
    return 0;
}

static int delta_sysfpga_i2c_smbus_xfer(struct i2c_adapter *adap, u16 addr, unsigned short flags,
                                        char read_write, u8 command, int size, union i2c_smbus_data *data)
{
    struct fpga_i2c_bus_s *adap_data = i2c_get_adapdata(adap);

    // flags would be set in this function and transmit to i2c_msg

    mutex_lock(&fpga_lock);
    //SYSFPGA_DEBUG("   adap->owner->name = %s", &adap->owner->name);
    SYSFPGA_DEBUG("   addr = %#X\t\t flags = %#X", addr, flags);
    SYSFPGA_DEBUG("   read_write = %d\t command = %#X\t\t size = %d", read_write, command, size);

    switch(size)
    {
        case I2C_SMBUS_QUICK:
            //write
            SYSFPGA_DEBUG("   ---I2C_SMBUS_QUICK---");
            break;
        case I2C_FUNC_SMBUS_BYTE:
            SYSFPGA_DEBUG("   ---I2C_FUNC_SMBUS_BYTE---");
            break;
        default:
            break;
    }
    mutex_unlock(&fpga_lock);
    return 0;
}

static u32 delta_sysfpga_i2c_functionality(struct i2c_adapter *adap)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_SMBUS_BLOCK_DATA;
}

static const struct i2c_algorithm delta_sysfpga_i2c_algorithm = {
    .master_xfer   = delta_sysfpga_i2c_master_xfer,
    //.smbus_xfer    = delta_sysfpga_i2c_smbus_xfer,
    .functionality = delta_sysfpga_i2c_functionality,
};

static int test_driver(struct device_driver *d, void *data)
{
	SYSFPGA_DEBUG("   test_driver: 1");
    struct i2c_driver *driver;
    driver = kzalloc(sizeof(struct i2c_driver), GFP_KERNEL);
    driver = to_i2c_driver(d);
    const unsigned short *address_list;
    address_list = driver->address_list;
    int i;
    SYSFPGA_DEBUG("   test_driver: 2");
    if (!driver->detect)
        SYSFPGA_DEBUG("   test_driver: 3");
    if (!address_list)
        SYSFPGA_DEBUG("   test_driver: 4");
    SYSFPGA_DEBUG("   test_driver: 5");
    kfree(driver);
    //return i2c_do_add_adapter(to_i2c_driver(d), data);
    return 0;
}

int delta_sysfpga_i2c_adapter_init(struct i2c_client *client, struct fpga_i2c_bus_s *fpga_data)
{
    //int num_i2c_master = 0;
    int current_bus = 0;
    int error = -1;

    memset(fpga_data, 0, sizeof(*fpga_data));
    fpga_data->adap = kzalloc(sizeof(struct i2c_adapter), GFP_KERNEL);
    //num_i2c_master = sizeof(sysfpga_i2c_info) / sizeof(sysfpga_i2c_s);
    //mutex_init(&fpga_lock);
    /* Check output */
    SYSFPGA_DEBUG("  client->adapter->name = %s", &client->adapter->name);
    SYSFPGA_DEBUG("  client->dev.kobj.name = %s", client->dev.kobj.name);
    SYSFPGA_DEBUG("  fpga_data->adap->nr = %d", fpga_data->adap->nr);
    SYSFPGA_DEBUG("  fpga_data->adap->dev.kobj.name = %s", fpga_data->adap->dev.kobj.name);
    SYSFPGA_DEBUG("  fpga_data->adap->owner.name = %s", &fpga_data->adap->owner->name);

    /* Create i2c adapter */
    snprintf(fpga_data->adap->name, sizeof(fpga_data->adap->name), sysfpga_i2c_info[current_bus].name, current_bus);
    fpga_data->adap->owner        = THIS_MODULE;
    fpga_data->adap->class        = I2C_CLASS_HWMON;
    //fpga_data->adap->retries      = 0;
    fpga_data->adap->algo         = &delta_sysfpga_i2c_algorithm;
    //fpga_data->adap->algo_data    = fpga_data;
    fpga_data->adap->dev.parent   = &client->dev;
    //fpga_data->adap->dev.of_node  = client->dev.of_node;
    fpga_data->offset             = sysfpga_i2c_info[current_bus].offset;
    i2c_set_adapdata(fpga_data->adap, fpga_data);
    
    SYSFPGA_DEBUG("  Add adapter");
    error = i2c_add_adapter(fpga_data->adap);
    if (error){
        SYSFPGA_DEBUG("i2c_add_adapter ERROR!");
        return error;
    }

    /* get driver */
    //SYSFPGA_DEBUG("   bus_for_each_drv");
    //bus_for_each_drv(client->adapter->dev.bus, NULL, client->adapter, test_driver);
    //bus_for_each_drv(fpga_data->adap->dev.bus, NULL, fpga_data->adap, test_driver);

    SYSFPGA_DEBUG("  client->adapter->name = %s", &client->adapter->name);
    SYSFPGA_DEBUG("  client->dev.kobj.name = %s", client->dev.kobj.name);
    SYSFPGA_DEBUG("  fpga_data->adap->nr = %d", fpga_data->adap->nr);
    SYSFPGA_DEBUG("  fpga_data->adap->dev.kobj.name = %s", fpga_data->adap->dev.kobj.name);
    SYSFPGA_DEBUG("  fpga_data->adap->owner.name = %s", &fpga_data->adap->owner->name);

    SYSFPGA_DEBUG("  delta_sysfpga_i2c_adapter_init OK");
    
    return 0;
}
//EXPORT_SYMBOL_GPL(delta_sysfpga_i2c_adapter_init);

int delta_sysfpga_i2c_adapter_exit(struct fpga_i2c_bus_s *fpga_data)
{
    if (fpga_data->adap) {
        i2c_del_adapter(fpga_data->adap);
        kfree(fpga_data->adap);
    }
    memset(fpga_data, 0, sizeof(*fpga_data));
    SYSFPGA_DEBUG("Delete i2c adapter");

    return 0;
}

MODULE_AUTHOR("Samantha Chu");
MODULE_DESCRIPTION("Delta SYSFPGA Driver");
MODULE_LICENSE("GPL");