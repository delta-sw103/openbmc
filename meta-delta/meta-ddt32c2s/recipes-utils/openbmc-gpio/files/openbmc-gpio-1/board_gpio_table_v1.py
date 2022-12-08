# fmt: off
# Copyright 2019-present Facebook. All rights reserved.
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

from openbmc_gpio_table import BoardGPIO

# The following table is generated using:
# python ddt32c2s_gpio_parser.py data/ddt32c2s-BMC-GPIO-DVT.csv
# DO NOT MODIFY THE TABLE!!!
# Manual modification will be overridden!!!

board_gpio_table_v1 = [
    BoardGPIO('GPIOF0', 'WP_FRU_EEPROM', 'low'),
    BoardGPIO('GPIOF1', 'INT_I210_BMC_ALERT_L', 'input'),
    BoardGPIO('GPIOG0', 'INT_FPGA_BMC_WDT_L', 'input'),
    BoardGPIO('GPIOG1', 'INT_FPGA_BMC_MON_L', 'input'),
    BoardGPIO('GPIOG4', 'RST_FPGA_BMC_SP_SW_L', 'input'),
    BoardGPIO('GPIOG5', 'RST_FPGA_BMC_LP_SW_L', 'input'),
    BoardGPIO('GPIOG7', 'RST_BMC_MUX_FAN_L', 'high'),
    BoardGPIO('GPIOQ1', 'RST_CPU_MB_PLTRST_SW_L', 'input'),
    BoardGPIO('GPIOQ2', 'RST_BMC_MB_STB_3V3', 'low'),
    BoardGPIO('GPIOQ3', 'BMC_BIOS_UPDATE_L', 'high'),
    BoardGPIO('GPIOQ4', 'INT_BMC_CPU_SHUTDOWN_L', 'high'),
    BoardGPIO('GPIOQ5', 'BMC_BIOS_SEL', 'high'),
    BoardGPIO('GPIOR0', 'EN_BMC_PS1_PSON_L', 'high'),
    BoardGPIO('GPIOR1', 'EN_BMC_PS2_PSON_L', 'high'),
    BoardGPIO('GPIOR3', 'INT_CPU_BMC_OVERHEAT_L', 'input'),
    BoardGPIO('GPIOR4', 'INT_BMC_CPU_IPMI_ALERT_L', 'high'),
    BoardGPIO('GPIOR5', 'INT_BMC_FPGA_DETECT', 'high'),
    BoardGPIO('GPIOY3', 'RST_BMC_RSTIC_FORCE_L', 'high'),
]
