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
SUMMARY = "Utilities"
DESCRIPTION = "Various utilities"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=eb723b61539feef013de476e68b5c50a"

SRC_URI = "file://COPYING \
           file://ast-functions \
           file://sol-util \
           file://sync_date.sh \
           file://power-on.sh \
           file://setup-eth.sh \
          "

pkgdir = "utils"

S = "${WORKDIR}"

# the tools for BMC will be installed in the image
binfiles = " sol-util sync_date.sh power-on.sh"

DEPENDS:append = "update-rc.d-native"
RDEPENDS:${PN} += "bash python3 "

do_install() {
  # install the package dir
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  install -d $dst

  # install ast-functions
  install -m 644 ast-functions ${dst}/ast-functions

  # create linkages to those binaries
  localbindir="${D}/usr/local/bin"
  install -d ${localbindir}
  for f in ${binfiles}; do
      install -m 755 $f ${dst}/${f}
      ln -s ../fbpackages/${pkgdir}/${f} ${localbindir}/${f}
  done
  
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d

  install -m 755 sync_date.sh ${D}${sysconfdir}/init.d/sync_date.sh
  update-rc.d -r ${D} sync_date.sh start 66 5 .

  install -m 755 setup-eth.sh ${D}${sysconfdir}/init.d/setup-eth.sh
  update-rc.d -r ${D} setup-eth.sh start 69 5 .

  install -m 755 power-on.sh ${D}${sysconfdir}/init.d/power-on.sh
  update-rc.d -r ${D} power-on.sh start 90 5 .
}

FILES:${PN} += "/usr/local ${sysconfdir}"
