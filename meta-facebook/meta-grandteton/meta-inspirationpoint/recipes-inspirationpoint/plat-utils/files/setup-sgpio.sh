#!/bin/bash
#
# Copyright 2014-present Facebook. All Rights Reserved.
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
# Provides:          sgpio setup
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: sgpio setup
### END INIT INFO

. /usr/local/fbpackages/utils/ast-functions

# input
sgpio_export PWRGD_OCP_SFF_PWR_GOOD 0
sgpio_export IRQ_SML1_PMBUS_ALERT_N 2
sgpio_export FM_PWRGD_PVDD18_S5_P0 4
sgpio_export P12V_HSC_TEMP_ALERT_R_N 6
sgpio_export GPU_FPGA_READY_ISO_R 8
sgpio_export APML_CPU0_ALERT_R_N 10
sgpio_export APML_CPU1_ALERT_R_N 12
sgpio_export FM_PVDD11_S3_P0_OCP_N 14
sgpio_export FM_PVDD11_S3_P1_OCP_N 16
sgpio_export P12V_HSC_T_CRIT_R_N 18
sgpio_export FM_SYS_THROTTLE_R_N 20
sgpio_export P12V_ALL_PWROK_R 22
sgpio_export HPDB_HSC_PWRGD_ISO_R 24
sgpio_export FM_UV_ADR_TRIGGER_R_N 26
sgpio_export PRSNT_OCP_SFF_N 28
sgpio_export PRSNT_OCP_V3_2_N 30
sgpio_export FM_SWB_BIC_READY_ISO_R_N 32
sgpio_export GPU_FPGA_EROT_FATALERR_ISO_R_N 34
sgpio_export IRQ_HSC_FAULT_R1_N 36
sgpio_export FM_OCP_V3_2_PWR_GOOD 38
sgpio_export GPU_FPGA_THERM_OVERT_ISO_R_N 40
sgpio_export GPU_FPGA_OVERT_ISO_R_N 42
sgpio_export FM_PWRGD_PVDD33_S5 44
sgpio_export BIC_MONITER_ISO_R 46
sgpio_export PWRGD_PVDD18_S5_R_P1 48
sgpio_export OCP_V3_2_P3V3_PWRGD 50
sgpio_export GPU_HMC_PRSNT_ISO_R_N 52
sgpio_export GPU_PRSNT_N_ISO_R 54
sgpio_export PWRGD_P3V3_R2 56
sgpio_export PWRGD_PVDDCR_SOC_P0 58
sgpio_export FM_GPU_PWRGD_ISO_R 60
sgpio_export RST_BMC_FROM_SWB_ISO_R_N 62
sgpio_export GPU_BASE_HMC_READY_ISO_R 64
sgpio_export PWRGD_PVDDCR_CPU0_P0 66
sgpio_export FM_PWRGD_PVDDIO_P0 68
sgpio_export PWRGD_P3V3_AUX_R 70
sgpio_export FM_PWRGD_PVDD11_S3_P0 72
sgpio_export FM_PWRGD_PVDDIO_P1 74
sgpio_export PWRGD_PS_PWROK_PLD_ISO_R 76
sgpio_export FM_PWRGD_PVDDCR_CPU0_P1 78
sgpio_export PWRGD_PVDDCR_SOC_P1 80
sgpio_export FM_PWRGD_PVDD11_S3_P1 82
sgpio_export FM_PWRGD_PVDDCR_CPU1_P1 84
sgpio_export FM_PWRGD_PVDDCR_CPU1_P0 86
sgpio_export PWRGD_P1V2_AUX_R 88
sgpio_export PWRGD_P1V8_AUX_R 90
sgpio_export SCM_SYS_PWROK 92
sgpio_export PWRGD_P5V_AUX_R3 94
sgpio_export E1S_0_P3V3_ADC_ALERT 96
sgpio_export P12V_SCM_ADC_ALERT 98
sgpio_export M2_0_P3V3_ADC_ALERT 100
sgpio_export OCP_V3_2_P3V3_ADC_ALERT 102
sgpio_export OCP_SFF_P3V3_ADC_ALERT 104
sgpio_export FM_UARTSW_LSB_N 106
sgpio_export FM_UARTSW_MSB_N 108
sgpio_export FM_THERMAL_ALERT_R_N 110
sgpio_export FM_PRSNT_CPU0_R_N 112
sgpio_export FM_PRSNT_CPU1_R_N 114
sgpio_export FM_SYS_THROTTLE_N_reg 116
sgpio_export CPU0_THERMTRIP_R_N 118
sgpio_export FM_THROTTLE_IN_N 120
sgpio_export FM_PVDDCR_CPU0_P0_OCP_N 122
sgpio_export FM_PVDDCR_CPU0_P1_OCP_N 124
sgpio_export FM_PVDDCR_CPU1_P0_OCP_N 126
sgpio_export FM_PVDDCR_CPU1_P1_OCP_N 128
sgpio_export PWRGD_CHGL_CPU0 130
sgpio_export PWRGD_CHAF_CPU0 132
sgpio_export PWRGD_CHAF_CPU1 134
sgpio_export CPU1_THERMTRIP_R_N 136
sgpio_export FM_CPU0_RSMRST_N 138
sgpio_export FM_SCM_PRSNT1_N 140
sgpio_export FM_CPU0_SMERR_N 142
sgpio_export FM_CPU1_SMERR_N 144
sgpio_export FM_BIOS_POST_CMPLT_BUF_R_N 146
sgpio_export SWB_HSC_PWRGD_ISO_R 148
sgpio_export FM_SMB_1_ALERT_GPU_ISO_R_N 150
sgpio_export FM_SMB_2_ALERT_GPU_ISO_R_N 152
sgpio_export PVDDCR_CPU0_P0_PMALERT 154
sgpio_export PVDDCR_CPU0_P1_PMALERT 156
sgpio_export PVDDCR_CPU1_P0_SMB_ALERT 158
sgpio_export PVDDCR_CPU1_P1_SMB_ALERT 160
sgpio_export FM_BMC_DBP_PRESENT_N 162
sgpio_export FM_CPU0_SLP_S3_N 164
sgpio_export FM_CPU0_SLP_S5_N 166
sgpio_export FM_CPU0_PROCHOT_R_N_reg 168
sgpio_export FM_CPU1_PROCHOT_R_N_reg 170
sgpio_export E1S_0_PRSNT_R_N 172
sgpio_export FM_SMB_RST_E1S_0_N 174
sgpio_export P12V_E1S_0_EN 176
sgpio_export OC_ALERT_N 178
sgpio_export PWRGD_CHGL_CPU1 180
sgpio_export PVDD11_S3_P0_PMALERT 182
sgpio_export PVDD11_S3_P1_PMALERT 184
sgpio_export FM_CPU1_PROCHOT_R_N 186
sgpio_export UV_ALERT_N 188
sgpio_export PCIE_M2_SSD0_PRSNT_N 190
sgpio_export FM_SWB_PWRGD_ISO_R 192
sgpio_export CPU0_PCC_N 194
sgpio_export CPU1_PCC_N 196
sgpio_export FM_RST_CPU0_RESETL_N 198
sgpio_export FM_RST_CPU1_RESETL_N 200
sgpio_export FM_CPU0_PROCHOT_R_N 202
sgpio_export FM_BOARD_BMC_SKU_ID4 212
sgpio_export FM_BOARD_BMC_SKU_ID3 214
sgpio_export FM_BOARD_BMC_SKU_ID2 216
sgpio_export FM_BOARD_BMC_SKU_ID1 218
sgpio_export FM_BOARD_BMC_SKU_ID0 220
sgpio_export FAB_BMC_REV_ID2 222
sgpio_export FAB_BMC_REV_ID1 224
sgpio_export FAB_BMC_REV_ID0 226
sgpio_export BMC_JTAG_SEL_R 228
sgpio_export CPLD_JTAG_EN 230
sgpio_export FM_CPU0_PWR_GOOD 232
sgpio_export FM_CPU1_PWR_GD_IN 234
sgpio_export FM_CPU1_PWRGD_OUT 236
sgpio_export FM_PWRGD_CPU0_PWROK 238
sgpio_export FM_PWRGD_CPU1_PWROK 240
sgpio_export PWRGD_P12V_MEM_R 242
sgpio_export P12V_SCM_PWRGD_R 244
sgpio_export P12V_E1S_0_PWRGD 246
sgpio_export P3V3_E1S_PWRGD_0_R 248
sgpio_export P12V_OCP_V3_1_PWRGD 250
sgpio_export OCP_V3_1_P3V3_PWRGD 252
sgpio_export HP_LVC3_OCP_V3_2_P12V_PWRGD_R 254

#output
sgpio_export FM_ESPI_PLD_R1_EN 1
gpio_set FM_ESPI_PLD_R1_EN 1

sgpio_export IRQ_BMC_CPU_NMI 3
gpio_set IRQ_BMC_CPU_NMI 1

sgpio_export RST_SMB_SWITCH_N 5
gpio_set RST_SMB_SWITCH_N 1

sgpio_export FM_SMM_CRASHDUMP_R 7
gpio_set FM_SMM_CRASHDUMP_R 1

sgpio_export FM_DEBUG_PORT_PRSNT_R1_N 9
gpio_set FM_DEBUG_PORT_PRSNT_R1_N 1

sgpio_export IRQ_PCH_SCI_WHEA_R_N 11
gpio_set IRQ_PCH_SCI_WHEA_R_N 1

sgpio_export FM_PECI_SEL_R_N 13
gpio_set FM_PECI_SEL_R_N 1

sgpio_export FM_JTAG_BMC_MUX_SEL_R1 15
gpio_set FM_JTAG_BMC_MUX_SEL_R1 1

sgpio_export FM_SPD_REMOTE_EN_R 17
gpio_set FM_SPD_REMOTE_EN_R 1

sgpio_export RST_BMC_RSTBTN_OUT_R_N 21
gpio_set RST_BMC_RSTBTN_OUT_R_N 1

sgpio_export IRQ_BMC_PCH_NMI_N 27
gpio_set IRQ_BMC_PCH_NMI_N 1

sgpio_export FM_BMC_ONCTL_R_N 35
gpio_set FM_BMC_ONCTL_R_N 1

sgpio_export FM_BMC_READY_R_N 37
gpio_set FM_BMC_READY_R_N 1

sgpio_export FM_BIOS_DEBUG_EN_R_N 41
gpio_set FM_BIOS_DEBUG_EN_R_N 1

sgpio_export GPU_NVS_PWR_BRAKE_R_N 43
gpio_set GPU_NVS_PWR_BRAKE_R_N 1

sgpio_export GPU_WP_HW_CTRL_R_N 45
gpio_set GPU_WP_HW_CTRL_R_N 1

sgpio_export CLK_GEN2_BMC_RC_OE_R_N 47
gpio_set CLK_GEN2_BMC_RC_OE_R_N 1

sgpio_export IRQ_SML0_ALERT_R_N 49
gpio_set IRQ_SML0_ALERT_R_N 1

sgpio_export BMC_MONITER 51
gpio_set BMC_MONITER 1

sgpio_export FM_PFR_DSW_PWROK_N 53
gpio_set FM_PFR_DSW_PWROK_N 1

sgpio_export RST_PFR_OVR_RTC_R 55
gpio_set RST_PFR_OVR_RTC_R 1

sgpio_export FM_BMC_TPM_PRES_N_R 57
gpio_set FM_BMC_TPM_PRES_N_R 1

sgpio_export FM_TPM_PRSNT_1_N 59
gpio_set FM_TPM_PRSNT_1_N 1

sgpio_export IRQ_PCH_TPM_SPI_N 61
gpio_set IRQ_PCH_TPM_SPI_N 1

sgpio_export USB_OC0_REAR_R_N 63
gpio_set USB_OC0_REAR_R_N 1

sgpio_export GPU_FPGA_RST_N 65
gpio_set GPU_FPGA_RST_N 1

sgpio_export FM_JTAG_TCK_MUX_BMC_SEL 67
gpio_set FM_JTAG_TCK_MUX_BMC_SEL 1

sgpio_export CLK_GEN2_GPU_FPGA_OE_R_N 69
gpio_set CLK_GEN2_GPU_FPGA_OE_R_N 1

sgpio_export FM_BMC_CPU_FBRK_OUT_N 71
gpio_set FM_BMC_CPU_FBRK_OUT_N 1

sgpio_export FM_REMOTE_DEBUG_EN_N 73
gpio_set FM_REMOTE_DEBUG_EN_N 1

sgpio_export RST_IOEXP_N 75
gpio_set RST_IOEXP_N 1

sgpio_export PMB_ALERT_EN_N 77
gpio_set PMB_ALERT_EN_N 1

sgpio_export FM_SPD_SWITCH_CTRL_N 79
gpio_set FM_SPD_SWITCH_CTRL_N 1

sgpio_export FM_SYS_RESET_N 81
gpio_set FM_SYS_RESET_N 1

sgpio_export Int_SCM_USB_OC_R_N 83
gpio_set Int_SCM_USB_OC_R_N 1

sgpio_export FM_SMB_RST_E1S_1_N 85
gpio_set FM_SMB_RST_E1S_1_N 1

sgpio_export RM_THROTTLE_EN_N 87
gpio_set RM_THROTTLE_EN_N 1

sgpio_export JTAG_SCM_TRST_R_N 91
gpio_set JTAG_SCM_TRST_R_N 1

sgpio_export BIOS_DEBUG_MODE_DIS 93
gpio_set BIOS_DEBUG_MODE_DIS 1

sgpio_export FM_BMC_PWRBRK_N 95
gpio_set FM_BMC_PWRBRK_N 1

sgpio_export BMC_PWRCAP_N 97
gpio_set BMC_PWRCAP_N 1

sgpio_export BMC_FORCE_NM_THROTTLE_N 99
gpio_set BMC_FORCE_NM_THROTTLE_N 1

sgpio_export FM_APML_MUX2_OE_R_N 101
gpio_set FM_APML_MUX2_OE_R_N 1

sgpio_export FM_APML_MUX2_SEL_R 103
gpio_set FM_APML_MUX2_SEL_R 1

sgpio_export FM_SEC_MUX_OE_R_N 105
gpio_set FM_SEC_MUX_OE_R_N 1

sgpio_export FM_SEC_MUX_R_SEL 107
gpio_set FM_SEC_MUX_R_SEL 1

sgpio_export IRQ_BMC_SMI_R_N 109
gpio_set IRQ_BMC_SMI_R_N 1

sgpio_export IRQ_CPU_BMC_NMI_R_N 111
gpio_set IRQ_CPU_BMC_NMI_R_N 1

sgpio_export IRQ_SMI_ACTIVE_N_R 113
gpio_set IRQ_SMI_ACTIVE_N_R 1

sgpio_export IRQ_TPM_SPI_R1_N 115
gpio_set IRQ_TPM_SPI_R1_N 1
