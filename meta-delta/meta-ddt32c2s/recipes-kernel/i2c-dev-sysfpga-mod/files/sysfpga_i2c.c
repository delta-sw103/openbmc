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

/* Only Port_PLD */
static const struct sysfpga_i2c_s sysfpga_i2c_info[] = {
    {"FPGA I2CBUS - PLD", I2C_PLD_CH, SYSFPGA_I2C_BASE_REG(I2C_PLD_CH)},
};

static int wait_complete(struct device *dev, uint32_t offset){
    uint32_t status = 0;
    int ret = 0, timeout = 0;

    while (!(status & GRABBER_READY_MASK)){
        if (timeout > SYSFPGA_I2C_TIMEOUT){
            SYSFPGA_DEBUG("i2c bus is busy: timeout. time spent: %d us", timeout);
            return -ETIMEDOUT;
        }
        ret = i2c_fpga_read(dev, GRAB_ENABLE(offset), &status);
        udelay(100);
        timeout += 100;
    }
    return 0;
}

static int check_fail_status(struct device *dev, uint32_t offset){
    uint32_t status = 0;
    int ret = 0;

    ret = i2c_fpga_read(dev, GRAB_FAIL_STATUS_1(offset), &status);
    if (!(status & GRABBER_FAIL_MASK)){
        return 0; //no fail
    }
    return -EIO;
}

static int delta_sysfpga_i2c_read(struct device *dev, uint32_t offset, uint32_t addr, uint32_t reg_addr, int data_len, uint8_t *buf)
{
    //SYSFPGA_DEBUG("---READ---");
    uint32_t ctrl_data = 0, addr_data = 0, rw_data = 0;
    int reg_length = 1;
    int ret = 0, i = 0;

    // Get write control
    ret = i2c_fpga_write(dev, FPGA_WRITE_CTRL, BMC_WRITE_CTRL);

    // Set reg address
    addr_data = reg_addr & 0xff;
    //SYSFPGA_DEBUG("\taddr_data = 0x%8X", addr_data);
    ret = i2c_fpga_write(dev, GRAB_ADDR(offset), addr_data);

    // Set control
    ctrl_data |= (addr & 0x7f) << CTRL_DEVICE_ADDR_OFFSET;
    ctrl_data |= (data_len & 0x1ff) << CTRL_DATA_LENGTH_OFFSET;
    ctrl_data |= (reg_length & 0x3) << CTRL_REGADDR_LENGTH_OFFSET;
    //SYSFPGA_DEBUG("\tctrl_data = 0x%8X", ctrl_data);
    ret = i2c_fpga_write(dev, GRAB_CTRL(offset), ctrl_data);

    // Set enable
    ret = i2c_fpga_write(dev, GRAB_ENABLE(offset), GRABBER_ENABLE_MASK);

    // Release write control
    ret = i2c_fpga_write(dev, FPGA_WRITE_CTRL, 0);

    // Check bus status
    ret = wait_complete(dev, offset);
    if (ret)
        return ret;
    
    // Check fail status
    ret = check_fail_status(dev, offset);
    if (ret)
        return ret;
    
    // Get data
    for (i=1; i<=data_len; i++){
        if ((i - 1) % 4 == 0){
            ret = i2c_fpga_read(dev, GRAB_DATA(offset) + (i - 1), &rw_data);
        }
        buf[i-1] = (uint8_t) (rw_data >> (((i - 1) % 4) * 8));
        //SYSFPGA_DEBUG("\tbuf[%d] = %#X", i-1, buf[i-1]);
    }

    return 0;
}

static int delta_sysfpga_i2c_write(struct device *dev, uint32_t offset, uint32_t addr, uint32_t reg_addr, int data_len, uint8_t *buf)
{
    //SYSFPGA_DEBUG("---WRITE---");
    uint32_t ctrl_data = 0, addr_data = 0, rw_data = 0;
    int reg_length = 1;
    int ret = 0, i = 0, j = 0;

    // Get write control
    ret = i2c_fpga_write(dev, FPGA_WRITE_CTRL, BMC_WRITE_CTRL);
    
    // Set data
    for (i=0; i<(data_len / 4); i++){
        rw_data = (buf[i * 4 + 3] << 24) | (buf[i * 4 + 2] << 16) | (buf[i * 4 + 1] << 8) | buf[i * 4 + 0];
        ret = i2c_fpga_write(dev, GRAB_DATA(offset) + (i * 4), rw_data);
        //SYSFPGA_DEBUG("\trw_data[%d] = 0x%8X", i, rw_data);
    }
    rw_data = 0;
    for (j=0; j<(data_len % 4); j++){
        rw_data |= (buf[i * 4 + j] << (j * 8));
    }
    ret = i2c_fpga_write(dev, GRAB_DATA(offset) + (i * 4), rw_data);
    //SYSFPGA_DEBUG("\trw_data[%d] = 0x%8X", i, rw_data);

    // Set reg address
    addr_data = reg_addr & 0xff;
    //SYSFPGA_DEBUG("\taddr_data = 0x%8X", addr_data);
    ret = i2c_fpga_write(dev, GRAB_ADDR(offset), addr_data);

    // Set control
    ctrl_data |= (addr & 0x7f) << CTRL_DEVICE_ADDR_OFFSET;
    ctrl_data |= (data_len & 0x1ff) << CTRL_DATA_LENGTH_OFFSET;
    ctrl_data |= (reg_length & 0x3) << CTRL_REGADDR_LENGTH_OFFSET;
    ctrl_data |= CTRL_RW_MASK;
    //SYSFPGA_DEBUG("\tctrl_data = 0x%8X", ctrl_data);
    ret = i2c_fpga_write(dev, GRAB_CTRL(offset), ctrl_data);

    // Set enable
    ret = i2c_fpga_write(dev, GRAB_ENABLE(offset), GRABBER_ENABLE_MASK);

    // Release write control
    ret = i2c_fpga_write(dev, FPGA_WRITE_CTRL, 0);

    // Check bus status
    ret = wait_complete(dev, offset);
    if (ret)
        return ret;
    
    // Check fail status
    ret = check_fail_status(dev, offset);
    if (ret)
        return ret;

    return 0;
}

static int delta_sysfpga_i2c_smbus_xfer(struct i2c_adapter *adap, u16 addr, unsigned short flags,
                                        char read_write, u8 command, int size, union i2c_smbus_data *data)
{
	int status;
    struct device *dev = adap->dev.parent;
    struct fpga_i2c_bus_s *adap_data = i2c_get_adapdata(adap);
    int offset = adap_data->offset;

    mutex_lock(&fpga_lock);

    /* Reference from /drivers/i2c/i2c-core-smbus.c i2c_smbus_xfer_emulated() */

	switch (size) {
	case I2C_SMBUS_QUICK:
        //SYSFPGA_DEBUG("---I2C_SMBUS_QUICK---");
        status = delta_sysfpga_i2c_write(dev, offset, addr, command, 0, NULL);
		break;
    
	case I2C_SMBUS_BYTE:
        //SYSFPGA_DEBUG("---I2C_SMBUS_BYTE---");
		if (read_write == I2C_SMBUS_READ){
			/* Special case: only a read! */
            status = delta_sysfpga_i2c_read(dev, offset, addr, command, 1, &data->byte);
		}
		break;
    
	case I2C_SMBUS_BYTE_DATA:
        //SYSFPGA_DEBUG("---I2C_SMBUS_BYTE_DATA---");
        if (&data->byte == NULL)
            return -EINVAL;
		if (read_write == I2C_SMBUS_READ){
            status = delta_sysfpga_i2c_read(dev, offset, addr, command, 1, &data->byte);
        }
		else {
            status = delta_sysfpga_i2c_write(dev, offset, addr, command, 1, &data->byte);
		}
		break;
    
	case I2C_SMBUS_WORD_DATA:
        //SYSFPGA_DEBUG("---I2C_SMBUS_WORD_DATA---");
        if (&data->word == NULL)
            return -EINVAL;
		if (read_write == I2C_SMBUS_READ){
            status = delta_sysfpga_i2c_read(dev, offset, addr, command, 2, (uint8_t *)&data->word);
        } else {
            status = delta_sysfpga_i2c_write(dev, offset, addr, command, 2, (uint8_t *)&data->word);
		}
		break;

	case I2C_SMBUS_PROC_CALL:
        //SYSFPGA_DEBUG("---I2C_SMBUS_PROC_CALL---");
		/* Special case */
		read_write = I2C_SMBUS_READ;
        status = delta_sysfpga_i2c_read(dev, offset, addr, command, 2, (uint8_t *)&data->word);
		break;
    
	case I2C_SMBUS_BLOCK_DATA:
        //SYSFPGA_DEBUG("---I2C_SMBUS_BLOCK_DATA---");
        if (&data->block[1] == NULL)
            return -EINVAL;
        if (data->block[0] > SYSFPGA_I2C_MAX_DATA_LENGTH) {
			dev_err(&adap->dev, "Invalid block %s size %d\n",
				read_write == I2C_SMBUS_READ ? "read" : "write",
				data->block[0]);
			return -EINVAL;
		}
		if (read_write == I2C_SMBUS_READ) {
            status = delta_sysfpga_i2c_read(dev, offset, addr, command, data->block[0]+1, &data->block[0]);
		} else {
            status = delta_sysfpga_i2c_write(dev, offset, addr, command, data->block[0]+1, &data->block[0]);
		}
		break;
    
	case I2C_SMBUS_BLOCK_PROC_CALL:
        //SYSFPGA_DEBUG("---I2C_SMBUS_BLOCK_PROC_CALL---");
		/* Another special case */
		read_write = I2C_SMBUS_READ;
		if (data->block[0] > SYSFPGA_I2C_MAX_DATA_LENGTH) {
			dev_err(&adap->dev,
				"Invalid block write size %d\n",
				data->block[0]);
			return -EINVAL;
		}
        status = delta_sysfpga_i2c_read(dev, offset, addr, command, data->block[0]+1, &data->block[0]);
		break;
    
	case I2C_SMBUS_I2C_BLOCK_DATA:
        //SYSFPGA_DEBUG("---I2C_SMBUS_I2C_BLOCK_DATA---");
        if (&data->block[1] == NULL)
            return -EINVAL;
		if (data->block[0] > SYSFPGA_I2C_MAX_DATA_LENGTH) {
			dev_err(&adap->dev, "Invalid block %s size %d\n",
				read_write == I2C_SMBUS_READ ? "read" : "write",
				data->block[0]);
			return -EINVAL;
		}
		if (read_write == I2C_SMBUS_READ) {
            status = delta_sysfpga_i2c_read(dev, offset, addr, command, data->block[0], &data->block[1]);
		} else {
            status = delta_sysfpga_i2c_write(dev, offset, addr, command, data->block[0], &data->block[1]);
		}
		break;
    
	default:
		dev_err(&adap->dev, "Unsupported transaction %d\n", size);
		return -EOPNOTSUPP;
        break;
	}

    mutex_unlock(&fpga_lock);
	return status;
}

static u32 delta_sysfpga_i2c_functionality(struct i2c_adapter *adap)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_SMBUS_BLOCK_DATA;
}

static const struct i2c_algorithm delta_sysfpga_i2c_algorithm = {
    .smbus_xfer    = delta_sysfpga_i2c_smbus_xfer,
    .functionality = delta_sysfpga_i2c_functionality,
};

int delta_sysfpga_i2c_adapter_init(struct i2c_client *client, struct fpga_i2c_bus_s *fpga_data)
{
    int current_bus = 0;
    int error = -1;

    mutex_init(&fpga_lock);

    memset(fpga_data, 0, sizeof(*fpga_data));
    fpga_data->adap = kzalloc(sizeof(struct i2c_adapter), GFP_KERNEL);

    /* Create i2c adapter */
    snprintf(fpga_data->adap->name, sizeof(fpga_data->adap->name), sysfpga_i2c_info[current_bus].name, current_bus);
    fpga_data->adap->owner        = THIS_MODULE;
    fpga_data->adap->class        = I2C_CLASS_SPD;
    fpga_data->adap->algo         = &delta_sysfpga_i2c_algorithm;
    fpga_data->adap->dev.parent   = &client->dev;
    fpga_data->offset             = sysfpga_i2c_info[current_bus].offset;
    i2c_set_adapdata(fpga_data->adap, fpga_data);
    
    error = i2c_add_adapter(fpga_data->adap);
    if (error){
        SYSFPGA_DEBUG("i2c_add_adapter ERROR!");
        return error;
    }
    printk("Add adapter: %s\n", fpga_data->adap->name);
    
    return 0;
}

int delta_sysfpga_i2c_adapter_exit(struct fpga_i2c_bus_s *fpga_data)
{
    if (fpga_data->adap) {
        printk("Delete i2c adapter: %s\n", fpga_data->adap->name);
        i2c_del_adapter(fpga_data->adap);
        kfree(fpga_data->adap);
    }
    memset(fpga_data, 0, sizeof(*fpga_data));
    return 0;
}