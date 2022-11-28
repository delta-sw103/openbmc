/*
 * delta_portpld.c - The i2c driver for DD-T32C2S PortPLD-A & B
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

#define PLD_DEBUG(fmt, ...) do {                   \
  printk(KERN_DEBUG "%s:%d " fmt "\n",            \
         __FUNCTION__, __LINE__, ##__VA_ARGS__);  \
} while (0)

#else /* !DEBUG */

#define PLD_DEBUG(fmt, ...)

#endif

static const i2c_dev_attr_st portpldA_attr_table[] = {
    {
        "port-cpld1-ver",
        NULL,
        I2C_DEV_ATTR_SHOW_DEFAULT, NULL,
        0x01, 0, 8,
    },
};

static const i2c_dev_attr_st portpldB_attr_table[] = {
    {
        "port-cpld2-ver",
        NULL,
        I2C_DEV_ATTR_SHOW_DEFAULT, NULL,
        0x01, 0, 8,
    },
};

static i2c_dev_data_st *portpldA_data;
static i2c_dev_data_st *portpldB_data;

/* PORTPLD i2c addresses. */
static const unsigned short normal_i2c[] = {0x41, 0x45, I2C_CLIENT_END};

/* PORTPLD id */
static const struct i2c_device_id portpld_id[] = {
    { "portpldA", 0 },
    { "portpldB", 0 },
    {},
};
MODULE_DEVICE_TABLE(i2c, portpld_id);

/* Return 0 if detection is successful, -ENODEV otherwise */
static int portpld_detect(struct i2c_client* client, struct i2c_board_info* info)
{
    /* We don't currently do any detection of PORTPLD */
    if (client->addr == 0x41)
        strlcpy(info->type, "portpldA", I2C_NAME_SIZE);
    if (client->addr == 0x45)
        strlcpy(info->type, "portpldB", I2C_NAME_SIZE);

    return 0;
}

static int portpld_probe(struct i2c_client* client, const struct i2c_device_id* id)
{
    struct device *dev = &client->dev;
    
    if (client->addr == 0x41){
        int n_attrs = sizeof(portpldA_attr_table) / sizeof(portpldA_attr_table[0]);
        portpldA_data = devm_kzalloc(dev, sizeof(i2c_dev_data_st), GFP_KERNEL);
        if (!portpldA_data) {
            return -ENOMEM;
        }
        return i2c_dev_sysfs_data_init(client, portpldA_data, portpldA_attr_table, n_attrs);
    }
    else if (client->addr == 0x45){
        int n_attrs = sizeof(portpldB_attr_table) / sizeof(portpldB_attr_table[0]);
        portpldB_data = devm_kzalloc(dev, sizeof(i2c_dev_data_st), GFP_KERNEL);
        if (!portpldB_data) {
            return -ENOMEM;
        }
        return i2c_dev_sysfs_data_init(client, portpldB_data, portpldB_attr_table, n_attrs);
    }
    return 0;
}

static int portpld_remove(struct i2c_client* client)
{
    if (client->addr == 0x41){
        i2c_dev_sysfs_data_clean(client, portpldA_data);
    }
    else if (client->addr == 0x45){
        i2c_dev_sysfs_data_clean(client, portpldB_data);
    }
    return 0;
}

static struct i2c_driver portpld_driver = {
    .class = I2C_CLASS_HWMON,
    .driver =
        {
            .name = "portpld",
        },
    .probe = portpld_probe,
    .remove = portpld_remove,
    .id_table = portpld_id,
    .detect = portpld_detect,
    .address_list = normal_i2c,
};

module_i2c_driver(portpld_driver);

MODULE_AUTHOR("Samantha Chu");
MODULE_DESCRIPTION("Delta PortPLD Driver");
MODULE_LICENSE("GPL");
