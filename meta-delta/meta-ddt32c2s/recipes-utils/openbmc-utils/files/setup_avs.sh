#!/bin/sh
# shellcheck disable=SC2004,SC2039,SC2242,SC2086
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

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

# Get AVS from SWPLD1, reg 0x2e, 0x2f
AVS_HIGH=$(i2cget -f -y 7 0x32 0x2e | awk '{printf "0x%04x",$1}')
AVS_LOW=$(i2cget -f -y 7 0x32 0x2f  | awk '{printf "0x%04x",$1}')
AVS=$(( $(($AVS_LOW << 0)) | $(($AVS_HIGH << 8)) ))
AVS_DATA=$(printf "0x%04x" $AVS)

if [[ $AVS_DATA -lt 0x0C29 || $AVS_DATA -gt 0x0E4E ]]; then
    echo "Invalid AVS value (0x0C29 - 0x0E4E)."
    exit -1
fi

IR_MUX_BUS=5
IR35233_ADDR=0x40
# PMBUS_PAGE=0x00
PMBUS_VOUT_MODE=0x20
PMBUS_VOUT_COMMAND=0x21
GET_VOUT_MODE=$(i2cget -f -y $IR_MUX_BUS $IR35233_ADDR $PMBUS_VOUT_MODE)
GET_VOUT_COMMAND=$(i2cget -f -y $IR_MUX_BUS $IR35233_ADDR $PMBUS_VOUT_COMMAND w)


# check vout_mode
if [[ $GET_VOUT_MODE -ne 0x14 ]]; then
    echo "Set PMBUS VOUT MODE = 0x14"
    i2cset -f -y $IR_MUX_BUS $IR35233_ADDR $PMBUS_VOUT_MODE 0x14
fi

# check vout_command
if [[ $GET_VOUT_COMMAND -ne $AVS_DATA ]]; then
    echo "Set PMBUS VOUT COMMAND = $AVS_DATA"
    i2cset -f -y $IR_MUX_BUS $IR35233_ADDR $PMBUS_VOUT_COMMAND $AVS_DATA w
fi
