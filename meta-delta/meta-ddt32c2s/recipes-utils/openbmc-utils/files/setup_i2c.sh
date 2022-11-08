#!/bin/bash
# shellcheck disable=SC1091,SC2034
# Copyright 2020-present Delta Eletronics, Inc. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

. /usr/local/bin/openbmc-utils.sh

KERNEL_VERSION=$(uname -r)
start_of_mux_bus=16

get_mux_bus_num() {
    echo $((start_of_mux_bus + $1))
}

# Bus 0
i2c_device_add 0 0x50 24c02                 # PSU1 EEPROM
i2c_device_add 0 0x58 dps_driver            # PSU1 PSU_DRIVER
i2c_device_add 0 0x51 24c02                 # PSU2 EEPROM
i2c_device_add 0 0x59 dps_driver            # PSU2 PSU_DRIVER

# Bus 1
# i2c_device_add 1 0x49 tmp75               # Thermal RF
# i2c_device_add 1 0x4a tmp75               # Thermal LF
# i2c_device_add 1 0x4b tmp75               # Thermal Upper
# i2c_device_add 1 0x4e tmp75               # Thermal Lower

# Bus 2
i2c_device_add 2 0x2d emc2305               # Fan Controller
i2c_device_add 2 0x4c emc2305               # Fan Controller
i2c_device_add 2 0x4d emc2305               # Fan Controller
# i2c_device_add 2 0x4f tmp75               # Thermal Sensor

# Bus 3
i2c_device_add 3 0x36 sysfpga               # sys-FPGA

# Bus 4

# Bus 5
i2c_device_add 5 0x27 pca9555               # PCA9555
# i2c_device_add 5 0x72 pca9548             # PCA9548

# Bus 6

# Bus 7
i2c_device_add 7 0x50 24c02                 # BMC EEPROM

# i2c-mux 5-0072: child bus 16-23

# i2c-mux 5, channel 1
i2c_device_add 16 0x50 24c64     #FAN1 Tray
# i2c-mux 5, channel 2
i2c_device_add 17 0x50 24c64     #FAN2 Tray
# i2c-mux 5, channel 3
i2c_device_add 18 0x50 24c64     #FAN3 Tray
# i2c-mux 5, channel 4
i2c_device_add 19 0x50 24c64     #FAN4 Tray
# i2c-mux 5, channel 5
i2c_device_add 20 0x50 24c64     #FAN5 Tray
# i2c-mux 5, channel 6
i2c_device_add 21 0x50 24c64     #FAN6 Tray
# i2c-mux 5, channel 7
i2c_device_add 22 0x50 24c64     #FAN7 Tray


#
# Check if I2C devices are bound to drivers. A summary message (total #
# of devices and # of devices without drivers) will be dumped at the end
# of this function.
#
i2c_check_driver_binding
