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

inherit meson systemd 

SUMMARY = "IPMI Daemon"
DESCRIPTION = "Daemon to handle IPMI Messages."
SECTION = "base"
PR = "r1"
LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://ipmid.c;beginline=8;endline=20;md5=7783b537a8ff52cf362d3cdb4bb0f6e2"

LDFLAGS += " -lkv -lfruid -lipc -llog"
CFLAGS += "-Wall -Werror -pthread"

LOCAL_URI = " \
    file://ipmid.c \
    file://meson.build \
    file://timestamp.h \
    file://timestamp.c \
    file://sel.h \
    file://sel.c \
    file://fruid.h \
    file://fruid.c \
    file://lan.c \
    file://run-ipmid.sh \
    file://setup-ipmid.sh \
    file://ipmid.service \
    file://cfg-parse.h \
    file://cfg-parse.c \
    file://common-handles.h \
    file://common-handles.c \
    "

install_sysv() {
    install -d ${D}${sysconfdir}/init.d
    install -d ${D}${sysconfdir}/rcS.d
    install -d ${D}${sysconfdir}/sv
    install -d ${D}${sysconfdir}/sv/ipmid
    install -d ${D}${sysconfdir}/ipmid
    install -m 755 ${S}/setup-ipmid.sh ${D}${sysconfdir}/init.d/setup-ipmid.sh
    install -m 755 ${S}/run-ipmid.sh ${D}${sysconfdir}/sv/ipmid/run
    update-rc.d -r ${D} setup-ipmid.sh start 64 5 .
}

install_systemd() {
    install -d ${D}${systemd_system_unitdir}
    install -m 644 ${S}/ipmid.service ${D}${systemd_system_unitdir}
}

do_install() {
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  bin="${D}/usr/local/bin"
  install -d $dst
  install -d $bin
  install -m 755 ipmid ${dst}/ipmid
  ln -snf ../fbpackages/${pkgdir}/ipmid ${bin}/ipmid

  if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
      install_systemd
  else
      install_sysv
  fi
}

FBPACKAGEDIR = "${prefix}/local/fbpackages"

FILES:${PN} = "${FBPACKAGEDIR}/ipmid ${prefix}/local/bin ${sysconfdir} "

LDFLAGS += " -lm  -lobmc-i2c "
DEPENDS += " cjson libkv libfruid libipc libobmc-i2c libipmi libipmb libfruid update-rc.d-native"
RDEPENDS:${PN} += " cjson libfruid libipc libkv libipmi libipmb libfruid libobmc-i2c liblog "

binfiles = "ipmid"
pkgdir = "ipmid"

SYSTEMD_SERVICE:${PN} = "ipmid.service"
