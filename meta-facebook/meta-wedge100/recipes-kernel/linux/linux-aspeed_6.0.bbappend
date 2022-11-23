# Copyright 2020-present Facebook. All Rights Reserved.
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

FILESEXTRAPATHS:prepend := "${THISDIR}/patches:"

SRC_URI += "file://1001-hwmon-pmbus-bel-pfe-enable-PMBUS_SKIP_STATUS_CHECK-f.patch \
            file://1005-i2c-aspeed-update-ast2400-timing-settings.patch \
            file://1011-ARM-dts-aspeed-wedge100-Enable-spi1-devices.patch \
           "
