#!/bin/sh
# shellcheck disable=SC1091
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

### BEGIN INIT INFO
# Provides:          setup_board.sh
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Setup the board
### END INIT INFO

. /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

# Setup management port LED
# /usr/local/bin/setup_mgmt.sh led &
# /usr/local/bin/setup_mgmt.sh

# Force UART connect to BMC UART-5
# echo 0 > "$SWPLD1_SYSFS_DIR/console_sel"

# select lmsensor configuration
cp /etc/sensors.d/custom/ddt32c2s.conf /etc/sensors.d/ddt32c2s.conf


# export_gpio_pin for PCA9555 5-0027
gpiocli export -c 5-0027 -o 0 --shadow FAN1_PRESENT
gpiocli export -c 5-0027 -o 1 --shadow FAN2_PRESENT
gpiocli export -c 5-0027 -o 2 --shadow FAN3_PRESENT
gpiocli export -c 5-0027 -o 3 --shadow FAN4_PRESENT
gpiocli export -c 5-0027 -o 4 --shadow FAN5_PRESENT
gpiocli export -c 5-0027 -o 5 --shadow FAN6_PRESENT
gpiocli export -c 5-0027 -o 6 --shadow FAN7_PRESENT

# Setup eth0 ip address
ifconfig eth0 192.168.1.110
