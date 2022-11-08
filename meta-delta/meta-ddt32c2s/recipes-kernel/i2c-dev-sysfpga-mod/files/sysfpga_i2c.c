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

static int wait_complete(struct device *dev, uint32_t offset){
    uint32_t status = 0;
    int ret = 0, timeout = 0;

    while (!(status & SYSFPGA_I2C_READY_MASK)){
        if (timeout > SYSFPGA_I2C_TIMEOUT){
            SYSFPGA_DEBUG("i2c bus is busy: timeout. time spent: %d us", timeout);
            return -ETIMEDOUT;
        }
        ret = i2c_fpga_read(dev, CTRL_REG(offset), &status);
        udelay(100);
        timeout += 100;
    }
    return 0;
}

static int check_fail_status(struct device *dev, uint32_t offset){
    uint32_t status = 0;
    int ret = 0;

    ret = i2c_fpga_read(dev, CTRL_REG(offset), &status);
    //SYSFPGA_DEBUG("   status = 0x%8X", status);
    if (!(status & SYSFPGA_I2C_FAIL_STATUS_MASK)){
        return 0; //no fail
    }/*
    if (status & 0x80){
        SYSFPGA_DEBUG("Transaction failed: SDA Stuck");
    }else if (status & 0x40){
        SYSFPGA_DEBUG("Transaction failed: Arbitration Lost");
    }else if (status & 0x20){
        SYSFPGA_DEBUG("Transaction failed: I2C Master Stuck");
    }else if (status & 0x10){
        SYSFPGA_DEBUG("Transaction failed: I2C SCL isn't Ready");
    }else{
        SYSFPGA_DEBUG("Transaction failed: NACK Fail");
    }*/
    return -EIO;
}

static int delta_sysfpga_i2c_read(struct i2c_adapter *adap, uint32_t addr, uint32_t reg_addr, int data_len, uint8_t *buf)
{
    //SYSFPGA_DEBUG("---READ---");
    struct fpga_i2c_bus_s *adap_data = i2c_get_adapdata(adap);
    uint32_t ctrl_data = 0, addr_data = 0, rw_data = 0;
    int reg_length = 1, rw = 0; // rw = 0 -> read
    int ret = 0, i = 0;

    // Get write control
    ret = i2c_fpga_write(adap->dev.parent, FPGA_WRITE_CTRL, BMC_WRITE_CTRL);

    // Set reg address
    addr_data = reg_addr & 0xff;
    //SYSFPGA_DEBUG("\taddr_data = 0x%8X", addr_data);
    ret = i2c_fpga_write(adap->dev.parent, ADDR_REG(adap_data->offset), addr_data);

    // Set control
    ctrl_data |= ( addr & 0x7f ) << SYSFPGA_I2C_DEVICE_ADDR_OFFSET;
    ctrl_data |= ( data_len & 0x1ff ) << SYSFPGA_I2C_DATA_LENGTH_OFFSET;
    ctrl_data |= ( reg_length & 0x3 ) << SYSFPGA_I2C_REGADDR_LENGTH_OFFSET;
    ctrl_data |= ( rw & 0x1 ) << SYSFPGA_I2C_RW_OFFSET; // Read = 0
    ctrl_data |= 0x1 << SYSFPGA_I2C_START_OFFSET;   // Start
    //SYSFPGA_DEBUG("\tctrl_data = 0x%8X", ctrl_data);
    ret = i2c_fpga_write(adap->dev.parent, CTRL_REG(adap_data->offset), ctrl_data);

    // Check bus status
    ret = wait_complete(adap->dev.parent, adap_data->offset);
    if (ret)
        return ret;
    
    // Check fail status
    ret = check_fail_status(adap->dev.parent, adap_data->offset);
    if (ret)
        return ret;
    
    // Get data
    for (i=1; i<=data_len; i++){
        if ((i - 1) % 4 == 0){
            ret = i2c_fpga_read(adap->dev.parent, DATA_REG(adap_data->offset) + (i - 1), &rw_data);
        }
        buf[i-1] = (uint8_t) (rw_data >> (((i - 1) % 4) * 8));
        //SYSFPGA_DEBUG("\tbuf[%d] = %#X", i-1, buf[i-1]);
    }

    return 0;
}

static int delta_sysfpga_i2c_write(struct i2c_adapter *adap, uint32_t addr, uint32_t reg_addr, int data_len, uint8_t *buf)
{
    //SYSFPGA_DEBUG("---WRITE---");
    struct fpga_i2c_bus_s *adap_data = i2c_get_adapdata(adap);
    uint32_t ctrl_data = 0, addr_data = 0, rw_data = 0;
    int reg_length = 1, rw = 1; //rw = 1 -> write
    int ret = 0, i = 0, j = 0;

    // Get write control
    ret = i2c_fpga_write(adap->dev.parent, FPGA_WRITE_CTRL, BMC_WRITE_CTRL);
    
    // Set data
    for (i=0; i<(data_len / 4); i++){
        rw_data = (buf[i * 4 + 3] << 24) | (buf[i * 4 + 2] << 16) | (buf[i * 4 + 1] << 8) | buf[i * 4 + 0];
        ret = i2c_fpga_write(adap->dev.parent, DATA_REG(adap_data->offset) + (i * 4), rw_data);
        //SYSFPGA_DEBUG("\trw_data[%d] = 0x%8X", i, rw_data);
    }
    rw_data = 0;
    for (j=0; j<(data_len % 4); j++){
        rw_data |= (buf[i * 4 + j] << (j * 8));
    }
    ret = i2c_fpga_write(adap->dev.parent, DATA_REG(adap_data->offset) + (i * 4), rw_data);
    //SYSFPGA_DEBUG("\trw_data[%d] = 0x%8X", i, rw_data);

    // Set reg address
    addr_data = reg_addr & 0xff;
    //SYSFPGA_DEBUG("\taddr_data = 0x%8X", addr_data);
    ret = i2c_fpga_write(adap->dev.parent, ADDR_REG(adap_data->offset), addr_data);

    // Set control
    ctrl_data |= ( addr & 0x7f ) << SYSFPGA_I2C_DEVICE_ADDR_OFFSET;
    ctrl_data |= ( data_len & 0x1ff ) << SYSFPGA_I2C_DATA_LENGTH_OFFSET;
    ctrl_data |= ( reg_length & 0x3 ) << SYSFPGA_I2C_REGADDR_LENGTH_OFFSET;
    ctrl_data |= ( rw & 0x1 ) << SYSFPGA_I2C_RW_OFFSET; // Read = 0
    ctrl_data |= 0x1 << SYSFPGA_I2C_START_OFFSET;   // Start
    //SYSFPGA_DEBUG("\tctrl_data = 0x%8X", ctrl_data);
    ret = i2c_fpga_write(adap->dev.parent, CTRL_REG(adap_data->offset), ctrl_data);

    // Check bus status
    ret = wait_complete(adap->dev.parent, adap_data->offset);
    if (ret)
        return ret;
    
    // Check fail status
    ret = check_fail_status(adap->dev.parent, adap_data->offset);
    if (ret)
        return ret;

    return 0;
}

static int delta_sysfpga_i2c_master_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    int i = 0;
    SYSFPGA_DEBUG("    msgs->addr = %#X\t\t msgs->flags = %#X", msgs->addr, msgs->flags);
    SYSFPGA_DEBUG("    msgs->len  = %d\t\t num = %d", msgs->len, num);
    for (i=0; i<msgs->len; i++)
        SYSFPGA_DEBUG("    msgs->buf = %#X", msgs->buf[i]);
    return 0;
}

static int delta_sysfpga_i2c_smbus_xfer(struct i2c_adapter *adap, u16 addr, unsigned short flags,
                                        char read_write, u8 command, int size, union i2c_smbus_data *data)
{
    int i;

    mutex_lock(&fpga_lock);

    /* Reference from /drivers/i2c/i2c-core-smbus.c i2c_smbus_xfer_emulated() */
	unsigned char msgbuf0[I2C_SMBUS_BLOCK_MAX+3];
	unsigned char msgbuf1[I2C_SMBUS_BLOCK_MAX+2];
	int status;
    /* msg[0]: write, msg[1]: read */
	struct i2c_msg msg[2] = {
		{
			.addr = addr,
			.flags = flags,
			.len = 1,
			.buf = msgbuf0,
		}, {
			.addr = addr,
			.flags = flags | I2C_M_RD,
			.len = 0,
			.buf = msgbuf1,
		},
	};

	msgbuf0[0] = command;
	switch (size) {
	case I2C_SMBUS_QUICK:
        //SYSFPGA_DEBUG("---I2C_SMBUS_QUICK---");
        status = delta_sysfpga_i2c_write(adap, addr, command, 0, NULL);
		break;
    
	case I2C_SMBUS_BYTE:
        //SYSFPGA_DEBUG("---I2C_SMBUS_BYTE---");
		if (read_write == I2C_SMBUS_READ){
			/* Special case: only a read! */
            status = delta_sysfpga_i2c_read(adap, addr, command, 1, &data->byte);
		}
		break;
    
	case I2C_SMBUS_BYTE_DATA:
        //SYSFPGA_DEBUG("---I2C_SMBUS_BYTE_DATA---");
		if (read_write == I2C_SMBUS_READ){
            status = delta_sysfpga_i2c_read(adap, addr, command, 1, &data->byte);
        }
		else {
            status = delta_sysfpga_i2c_write(adap, addr, command, 1, &data->byte);
		}
		break;
    
	case I2C_SMBUS_WORD_DATA:
        SYSFPGA_DEBUG("---I2C_SMBUS_WORD_DATA---");
        break; // delete
		if (read_write == I2C_SMBUS_READ){
			msg[1].len = 2;
            
            status = delta_sysfpga_i2c_read(adap, addr, command, 2, (uint8_t *)&data->word);

            data->word = msgbuf1[0] | (msgbuf1[1] << 8);
        } else {
			msg[0].len = 3;
			msgbuf0[1] = data->word & 0xff;
			msgbuf0[2] = data->word >> 8;

            status = delta_sysfpga_i2c_write(adap, addr, command, 2, (uint8_t *)&data->word);

		}
		break;

	case I2C_SMBUS_PROC_CALL:
        SYSFPGA_DEBUG("---I2C_SMBUS_PROC_CALL---");
        break; // delete
		/* Special case */
		read_write = I2C_SMBUS_READ;
		msg[0].len = 3;
		msg[1].len = 2;
		msgbuf0[1] = data->word & 0xff;
		msgbuf0[2] = data->word >> 8;

        status = delta_sysfpga_i2c_read(adap, addr, command, 2, (uint8_t *)&data->word);

        data->word = msgbuf1[0] | (msgbuf1[1] << 8);
		break;
    
	case I2C_SMBUS_BLOCK_DATA:
        SYSFPGA_DEBUG("---I2C_SMBUS_BLOCK_DATA---");
        break; // delete
		if (read_write == I2C_SMBUS_READ) {
			msg[1].flags |= I2C_M_RECV_LEN;
			msg[1].len = 1; /* block length will be added by the underlying bus driver */
			//i2c_smbus_try_get_dmabuf(&msg[1], 0);

            status = delta_sysfpga_i2c_read(adap, addr, command, data->block[0]+1, &data->block[0]);

            if (msg[1].buf[0] > I2C_SMBUS_BLOCK_MAX) {
				dev_err(&adap->dev,
					"Invalid block size returned: %d\n",
					msg[1].buf[0]);
				status = -EPROTO;
				goto cleanup;
			}
			for (i = 0; i < msg[1].buf[0] + 1; i++)
				data->block[i] = msg[1].buf[i];
		} else {
			msg[0].len = data->block[0] + 2;
			if (msg[0].len > I2C_SMBUS_BLOCK_MAX + 2) {
				dev_err(&adap->dev,
					"Invalid block write size %d\n",
					data->block[0]);
				return -EINVAL;
			}

			//i2c_smbus_try_get_dmabuf(&msg[0], command);
			for (i = 1; i < msg[0].len; i++)
				msg[0].buf[i] = data->block[i - 1];
            
            status = delta_sysfpga_i2c_write(adap, addr, command, data->block[0]+1, &data->block[0]);

		}
		break;
    
	case I2C_SMBUS_BLOCK_PROC_CALL:
        SYSFPGA_DEBUG("---I2C_SMBUS_BLOCK_PROC_CALL---");
        break; // delete
		/* Another special case */
		read_write = I2C_SMBUS_READ;
		if (data->block[0] > I2C_SMBUS_BLOCK_MAX) {
			dev_err(&adap->dev,
				"Invalid block write size %d\n",
				data->block[0]);
			return -EINVAL;
		}

		msg[0].len = data->block[0] + 2;
		//i2c_smbus_try_get_dmabuf(&msg[0], command);
		for (i = 1; i < msg[0].len; i++)
			msg[0].buf[i] = data->block[i - 1];

		msg[1].flags |= I2C_M_RECV_LEN;
		msg[1].len = 1; /* block length will be added by
				   the underlying bus driver */
		//i2c_smbus_try_get_dmabuf(&msg[1], 0);

        status = delta_sysfpga_i2c_read(adap, addr, command, data->block[0]+1, &data->block[0]);

        if (msg[1].buf[0] > I2C_SMBUS_BLOCK_MAX) {
				dev_err(&adap->dev,
					"Invalid block size returned: %d\n",
					msg[1].buf[0]);
				status = -EPROTO;
				goto cleanup;
			}
			for (i = 0; i < msg[1].buf[0] + 1; i++)
				data->block[i] = msg[1].buf[i];
		break;
    
	case I2C_SMBUS_I2C_BLOCK_DATA:
        SYSFPGA_DEBUG("---I2C_SMBUS_I2C_BLOCK_DATA---");
        break; // delete
		if (data->block[0] > I2C_SMBUS_BLOCK_MAX) {
			dev_err(&adap->dev, "Invalid block %s size %d\n",
				read_write == I2C_SMBUS_READ ? "read" : "write",
				data->block[0]);
			return -EINVAL;
		}

		if (read_write == I2C_SMBUS_READ) {
			msg[1].len = data->block[0];
			//i2c_smbus_try_get_dmabuf(&msg[1], 0);

            status = delta_sysfpga_i2c_read(adap, addr, command, data->block[0], &data->block[1]);

            for (i = 0; i < data->block[0]; i++)
				data->block[i + 1] = msg[1].buf[i];
		} else {
			msg[0].len = data->block[0] + 1;

			//i2c_smbus_try_get_dmabuf(&msg[0], command);
			for (i = 1; i <= data->block[0]; i++)
				msg[0].buf[i] = data->block[i];
            
            status = delta_sysfpga_i2c_write(adap, addr, command, data->block[0], &data->block[1]);

		}
		break;
    
	default:
		dev_err(&adap->dev, "Unsupported transaction %d\n", size);
		return -EOPNOTSUPP;
        break;
	}

    /* if PEC */
	i = ((flags & I2C_CLIENT_PEC) && size != I2C_SMBUS_QUICK && size != I2C_SMBUS_I2C_BLOCK_DATA);
	if (i)
		SYSFPGA_DEBUG("-----PEC-----");

cleanup:
	if (msg[0].flags & I2C_M_DMA_SAFE)
		kfree(msg[0].buf);
	if (msg[1].flags & I2C_M_DMA_SAFE)
		kfree(msg[1].buf);

    mutex_unlock(&fpga_lock);
	return status;
}

static u32 delta_sysfpga_i2c_functionality(struct i2c_adapter *adap)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_SMBUS_BLOCK_DATA;
}

static const struct i2c_algorithm delta_sysfpga_i2c_algorithm = {
    //.master_xfer   = delta_sysfpga_i2c_master_xfer,
    .smbus_xfer    = delta_sysfpga_i2c_smbus_xfer,
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

    mutex_init(&fpga_lock);

    memset(fpga_data, 0, sizeof(*fpga_data));
    fpga_data->adap = kzalloc(sizeof(struct i2c_adapter), GFP_KERNEL);
    //num_i2c_master = sizeof(sysfpga_i2c_info) / sizeof(sysfpga_i2c_s);

    /* Create i2c adapter */
    snprintf(fpga_data->adap->name, sizeof(fpga_data->adap->name), sysfpga_i2c_info[current_bus].name, current_bus);
    fpga_data->adap->owner        = THIS_MODULE;
    //fpga_data->adap->class        = I2C_CLASS_HWMON;
    //fpga_data->adap->retries      = 0;
    fpga_data->adap->algo         = &delta_sysfpga_i2c_algorithm;
    //fpga_data->adap->algo_data    = fpga_data;
    fpga_data->adap->dev.parent   = &client->dev;
    //fpga_data->adap->dev.of_node  = client->dev.of_node;
    fpga_data->offset             = sysfpga_i2c_info[current_bus].offset;
    i2c_set_adapdata(fpga_data->adap, fpga_data);
    
    SYSFPGA_DEBUG("  Add adapter %s", fpga_data->adap->name);
    error = i2c_add_adapter(fpga_data->adap);
    if (error){
        SYSFPGA_DEBUG("i2c_add_adapter ERROR!");
        return error;
    }

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
        SYSFPGA_DEBUG("Delete i2c adapter %s", fpga_data->adap->name);
        i2c_del_adapter(fpga_data->adap);
        kfree(fpga_data->adap);
    }
    memset(fpga_data, 0, sizeof(*fpga_data));
    return 0;
}

MODULE_AUTHOR("Samantha Chu");
MODULE_DESCRIPTION("Delta SYSFPGA Driver");
MODULE_LICENSE("GPL");