#!/bin/sh
# shellcheck disable=SC1091
#
# Copyright 2019-present Facebook. All Rights Reserved.
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
. /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

FC1_SYSFS_DIR=$(i2c_device_sysfs_abspath 2-002d)
FC2_SYSFS_DIR=$(i2c_device_sysfs_abspath 2-004c)
FC3_SYSFS_DIR=$(i2c_device_sysfs_abspath 2-004d)

usage() {
    echo "Usage: $0 [Fan Unit (1..7)]" >&2
}

# FCB    PWM 1 ~ 14 control Fantray 1 2 3 4 5 6 7
show_pwm()
{
    if [ "$1" -eq 1 ] || [ "$1" -eq 2 ]; then
        front_pwm="$FC1_SYSFS_DIR/pwm$((($1 * 2 - 1)))"
        rear_pwm="$FC1_SYSFS_DIR/pwm$((($1 * 2)))"
    elif [ "$1" -eq 3 ]; then
        front_pwm="$FC1_SYSFS_DIR/pwm5"
        rear_pwm="$FC2_SYSFS_DIR/pwm1"
    elif [ "$1" -eq 4 ] || [ "$1" -eq 5 ]; then
        front_pwm="$FC2_SYSFS_DIR/pwm$((($1 * 2 - 6)))"
        rear_pwm="$FC2_SYSFS_DIR/pwm$((($1 * 2 - 5)))"
    else
        front_pwm="$FC3_SYSFS_DIR/pwm$((($1 * 2 - 11)))"
        rear_pwm="$FC3_SYSFS_DIR/pwm$((($1 * 2 - 10)))"
    fi

    front_val=$(head -n 1 "$front_pwm")
    rear_val=$(head -n 1 "$rear_pwm")


    echo "$((front_val * 100 / 255))%, $((rear_val * 100 / 255))%"

}

show_rpm()
{
    if [ "$1" -eq 1 ] || [ "$1" -eq 2 ]; then
        front_rpm="$FC1_SYSFS_DIR/fan$((($1 * 2 - 1)))_input"
        rear_rpm="$FC1_SYSFS_DIR/fan$((($1 * 2)))_input"
    elif [ "$1" -eq 3 ]; then
        front_rpm="$FC1_SYSFS_DIR/fan5_input"
        rear_rpm="$FC2_SYSFS_DIR/fan1_input"
    elif [ "$1" -eq 4 ] || [ "$1" -eq 5 ]; then
        front_rpm="$FC2_SYSFS_DIR/fan$((($1 * 2 - 6)))_input"
        rear_rpm="$FC2_SYSFS_DIR/fan$((($1 * 2 - 5)))_input"
    else
        front_rpm="$FC3_SYSFS_DIR/fan$((($1 * 2 - 11)))_input"
        rear_rpm="$FC3_SYSFS_DIR/fan$((($1 * 2 - 10)))_input"
    fi

    echo "$(cat "$front_rpm"), $(cat "$rear_rpm")"
}

set -e

# refer to the comments in init_pwn.sh regarding
# the fan unit and tacho mapping
if [ "$#" -eq 0 ]; then
    FANS="1 2 3 4 5 6 7"
elif [ "$#" -eq 1 ]; then
    if [ "$1" -le 0 ] || [ "$1" -ge 8 ]; then
        echo "Fan $1: not a valid Fan Unit"
        exit 1
    fi
    FANS="$1"
else
    usage
    exit 1
fi


for fan in $FANS; do
    FAN="${fan}"
    UNIT=${FAN}

    echo "Fan $fan RPMs: $(show_rpm "$UNIT"), ($(show_pwm "$UNIT"))"
done
