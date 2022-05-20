#!/bin/sh
# shellcheck disable=SC2086,SC2034,SC1091
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

fcb_ver=$(head -n1 "$FCMCPLD_SYSFS_DIR/cpld_ver" 2> /dev/null)

usage() {
    echo "Usage: $0 <PERCENT (0..100)> <Fan Unit (1..7)|all> <front|rear|both>" >&2
}

get_front_pwm() {
    if [ "$1" -eq 1 ] || [ "$1" -eq 2 ] || [ "$1" -eq 3 ]; then
        front_pwm="$FC1_SYSFS_DIR/pwm$((($1 * 2 - 1)))"
    elif [ "$1" -eq 4 ] || [ "$1" -eq 5 ]; then
        front_pwm="$FC2_SYSFS_DIR/pwm$((($1 * 2 - 6)))"
    else
        front_pwm="$FC3_SYSFS_DIR/pwm$((($1 * 2 - 11)))"
    fi

    echo $front_pwm
}

get_rear_pwm() {
    if [ "$1" -eq 1 ] || [ "$1" -eq 2 ]; then
        rear_pwm="$FC1_SYSFS_DIR/pwm$((($1 * 2)))"
    elif [ "$1" -eq 3 ] || [ "$1" -eq 4 ] || [ "$1" -eq 5 ]; then
        rear_pwm="$FC2_SYSFS_DIR/pwm$((($1 * 2 - 5)))"
    else
        rear_pwm="$FC3_SYSFS_DIR/pwm$((($1 * 2 - 10)))"
    fi

    echo $rear_pwm
}

set -e

if [ "$#" -ne 3 ]; then
    usage
    exit 1
fi

# FCB    PWM 1 ~ 14 control Fantray 1 2 3 4 5 6 7
if [ "$2" = "all" ]; then
    FAN_UNIT="1 2 3 4 5 6 7"
else
    if [ "$2" -le 0 ] || [ "$2" -ge 8 ]; then
        echo "Fan $2: not a valid Fan Unit"
        exit 1
    fi

    FAN="$2"
    FAN_UNIT=$((FAN / 1))
fi

step=$((($1 * 255) / 100))

cnt=1

for unit in ${FAN_UNIT}; do
    if [ "$3" = "front" ] || [ "$3" = "both" ];then
        front_pwm=$(get_front_pwm $unit)
        echo "$step" > "${front_pwm}"
        echo "Successfully set fan $unit front speed to $1%"
    fi

    if [ "$3" = "rear" ] || [ "$3" = "both" ];then
        rear_pwm=$(get_rear_pwm $unit)
        echo "$step" > "${rear_pwm}"
        echo "Successfully set fan $unit rear speed to $1%"
    fi
done
