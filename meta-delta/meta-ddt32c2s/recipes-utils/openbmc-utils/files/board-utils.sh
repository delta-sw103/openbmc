#!/bin/bash
# shellcheck disable=SC2034,SC1091
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
#

# Do not change this line to openbmc-utils.sh, or it will generate a source loop.
. /usr/local/bin/i2c-utils.sh

SYSFPGA_SYSFS_DIR=$(i2c_device_sysfs_abspath 3-0036)

# RESET
# VR_3V3_RST_SYSFS="${SWPLD1_SYSFS_DIR}/vr_3v3_rst"


# CPLD version
# SWPLD1_BOARD_ID=$(head -n 1 "$SWPLD1_SYSFS_DIR/board_id" 2> /dev/null)
# SWPLD1_BOARD_VER=$(head -n 1 "$SWPLD1_SYSFS_DIR/board_ver" 2> /dev/null)
SYSFPGA_VER=$(head -n 1 "$SYSFPGA_SYSFS_DIR/fpga_ver" 2> /dev/null)

# PSU
PSU1_PRESENT=$(head -n 1 "$SYSFPGA_SYSFS_DIR/psu1_present" 2> /dev/null)
PSU2_PRESENT=$(head -n 1 "$SYSFPGA_SYSFS_DIR/psu2_present" 2> /dev/null)
PSU1_ENABLE=$(head -n 1 "$SYSFPGA_SYSFS_DIR/psu1_enable" 2> /dev/null)
PSU2_ENABLE=$(head -n 1 "$SYSFPGA_SYSFS_DIR/psu2_enable" 2> /dev/null)
PSU1_PWR_OK=$(head -n 1 "$SYSFPGA_SYSFS_DIR/psu1_pwr_ok" 2> /dev/null)
PSU2_PWR_OK=$(head -n 1 "$SYSFPGA_SYSFS_DIR/psu2_pwr_ok" 2> /dev/null)


# Called by power-on.sh, wedge_power.sh
wedge_is_us_on() {
    return 0
}

# Called by setup_i2c.sh
wedge_board_type_rev(){
    echo "DD-T32C2S"
}

# Called by setup_default_gpio.sh
delta_board_rev() {
    echo "$SWPLD1_BOARD_VER"
}

delta_prepare_cpld_update() {
    echo "Stop fscd service."
    sv stop fscd

    echo "Disable watchdog."
    /usr/local/bin/wdtcli stop

    echo "Set fan speed 40%."
    set_fan_speed.sh 40 all both
}