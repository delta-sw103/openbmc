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
    echo "Usage: $0 <Mode> <Speed> <Fan Unit> <front|rear|both>" >&2
    echo "      Mode        pwm|rpm" >&2
    echo "      Speed       pwm: 0-100  ex. 50" >&2
    echo "                  rpm: real speed  ex. 13000" >&2
    echo "      Fan Unit    (1..7)|all" >&2
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

get_front_rpm() {
    if [ "$1" -eq 1 ] || [ "$1" -eq 2 ] || [ "$1" -eq 3 ]; then
        front_rpm="$FC1_SYSFS_DIR/fan$((($1 * 2 - 1)))_target"
    elif [ "$1" -eq 4 ] || [ "$1" -eq 5 ]; then
        front_rpm="$FC2_SYSFS_DIR/fan$((($1 * 2 - 6)))_target"
    else
        front_rpm="$FC3_SYSFS_DIR/fan$((($1 * 2 - 11)))_target"
    fi

    echo $front_rpm
}

get_rear_rpm() {
    if [ "$1" -eq 1 ] || [ "$1" -eq 2 ]; then
        rear_rpm="$FC1_SYSFS_DIR/fan$((($1 * 2)))_target"
    elif [ "$1" -eq 3 ] || [ "$1" -eq 4 ] || [ "$1" -eq 5 ]; then
        rear_rpm="$FC2_SYSFS_DIR/fan$((($1 * 2 - 5)))_target"
    else
        rear_rpm="$FC3_SYSFS_DIR/fan$((($1 * 2 - 10)))_target"
    fi

    echo $rear_rpm
}

set -e

if [ "$#" -ne 4 ]; then
    usage
    exit 1
fi

# FCB    1 ~ 14 control Fantray 1 2 3 4 5 6 7
if [ "$3" = "all" ]; then
    FAN_UNIT="1 2 3 4 5 6 7"
else
    if [ "$3" -le 0 ] || [ "$3" -ge 8 ]; then
        echo "Fan $3: not a valid Fan Unit"
        exit 1
    fi

    FAN="$3"
    FAN_UNIT=$((FAN / 1))
fi

# Check mode
if [ "$1" = "pwm" ]; then
    step=$((($2 * 255) / 100))
elif [ "$1" = "rpm" ]; then
    step=$2;
else
    echo "Wrong mode. Only pwm or rpm."
    exit 1
fi

cnt=1

# Set speed
for unit in ${FAN_UNIT}; do
    if [ "$4" = "front" ] || [ "$4" = "both" ];then
        if [ "$1" = "pwm" ]; then
            front_pwm=$(get_front_pwm $unit)
            echo "$step" > "${front_pwm}"
            echo "Successfully set fan $unit front speed to $2%"
        elif [ "$1" = "rpm" ]; then
            front_rpm=$(get_front_rpm $unit)
            echo "$step" > "${front_rpm}"
            echo "Successfully set fan $unit front speed to $2"
        fi
    fi

    if [ "$4" = "rear" ] || [ "$4" = "both" ];then
        if [ "$1" = "pwm" ]; then
            rear_pwm=$(get_rear_pwm $unit)
            echo "$step" > "${rear_pwm}"
            echo "Successfully set fan $unit rear speed to $2%"
        elif [ "$1" = "rpm" ]; then
            rear_rpm=$(get_rear_rpm $unit)
            echo "$step" > "${rear_rpm}"
            echo "Successfully set fan $unit rear speed to $2"
        fi
    fi
done
