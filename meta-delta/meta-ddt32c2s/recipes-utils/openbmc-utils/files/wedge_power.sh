#!/bin/bash
# shellcheck disable=SC1091
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

source /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

prog="$0"

PWR_CTRL_SYSFS="${SYSFPGA_SYSFS_DIR}/system_pwr"
PWR_CPU_SYSFS="${SYSFPGA_SYSFS_DIR}/cpu_disable"
PWR_MB_SYSFS="${SYSFPGA_SYSFS_DIR}/mb_pwr_enable"

usage(){
    echo "Usage: $prog <command> [command options]"
    echo
    echo "Commands:"
    echo "  status: Get the current microserver power status"
    echo
    echo "  on: Power on system if not powered on already"
    echo
    echo "  off: Power off system ungracefully"
    echo
    echo "  reset: Power reset microserver ungracefully"
    echo "    options:"
    echo "      -s: Power reset whole ddt32c2s system ungracefully"
    echo
}

do_status(){
    echo -n "Microserver power is "

    if [ $(wedge_is_us_on) ]; then
        echo "on"
    else
        echo "off"
    fi

    return 0
}

do_on(){
    local ret
    if [ $(wedge_is_us_on) ]; then
        echo "Already On. Skip!"
        return 1
    fi

    echo -n "Power on system ..."
    # Disable hold off - Keep in normal operation
    echo 1 > "$PWR_CTRL_SYSFS"
    ret=$?
    if [ $ret -eq 0 ]; then
        echo " Done"
    else
        echo " Failed"
    fi
    return $ret
}

do_off(){
    local ret
    if [ !$(wedge_is_us_on) ]; then
        echo "Already Off. Skip!"
        return 1
    fi

    echo -n "Power off system ..."
    # Hold Off - Disable CPU 3V3, MB_PWR, PSU_1 & PSU_2
    echo 0 > "$PWR_CTRL_SYSFS"
    ret=$?
    if [ $ret -eq 0 ]; then
        echo " Done"
    else
        echo " Failed"
    fi
    return $ret
}

do_reset(){
    local system ret
    if [ !$(wedge_is_us_on) ]; then
        echo "Power resetting microserver that is powered off has no effect."
        echo "Use '$prog on' to power the microserver on."
        return 1
    fi
    system=0
    while getopts "s" opt; do
        case $opt in
            s)
                system=1
                ;;
            *)
                usage
                exit 1
                ;;
        esac
    done
    if [ $system -eq 0 ]; then
        echo -n "Power reset microserver ..."
        echo 1 > "$PWR_CPU_SYSFS"
        ret=$?
        if [ $ret -eq 0 ]; then
            echo " Done. Wait for microserver reset."
        else
            echo " Failed"
        fi
    else
        echo -n "Power reset system ..."
        echo 0 > "$PWR_MB_SYSFS"
        ret=$?
        if [ $ret -eq 0 ]; then
            echo " Done. Wait for system reset."
        else
            echo " Failed"
        fi
    fi
    return $ret
}


if [ $# -lt 1 ]; then
    usage
    exit 1
fi

command="$1"
shift

case "$command" in
    status)
        do_status "$@"
        ;;
    on)
        if [ "$((SYSFPGA_VER))" -lt 8 ]; then
            echo -n "Error: "
            echo "The sys-FPGA version is too low to execute this function. Please update to version 8 or higher."
            echo "Your version: $SYSFPGA_VER"
            exit 0
        fi
        do_on "$@"
        ;;
    off)
        if [ "$((SYSFPGA_VER))" -lt 8 ]; then
            echo -n "Error: "
            echo "The sys-FPGA version is too low to execute this function. Please update to version 8 or higher."
            echo "Your version: $SYSFPGA_VER"
            exit 0
        fi
        do_off "$@"
        ;;
    reset)
        do_reset "$@"
        ;;
    *)
        usage
        exit 1
        ;;
esac

exit $?
