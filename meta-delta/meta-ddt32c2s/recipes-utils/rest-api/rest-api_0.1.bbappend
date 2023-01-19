# Copyright 2015-present Facebook. All Rights Reserved.
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
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

LOCAL_URI += " \
    file://rest.cfg \
    file://acl_config.py \
    file://acl_providers/ddt32c2s_acl_provider.py \
    file://ca/CA-cert.pem \
    file://ca/server-cert.pem \
    file://ca/server-key.pem \
"

aclfiles += "ddt32c2s_acl_provider.py"

binfiles += "acl_config.py"

do_install:append() {
    install -d ${D}${sysconfdir}
    install -d ${D}${sysconfdir}/ssl
    install -d ${D}${sysconfdir}/ssl/certs
    install -d ${D}${sysconfdir}/ssl/private
    install -m 644 ca/CA-cert.pem ${D}${sysconfdir}/ssl/certs/CA-cert.pem
    install -m 644 ca/server-cert.pem ${D}${sysconfdir}/ssl/certs/server-cert.pem
    install -m 400 ca/server-key.pem ${D}${sysconfdir}/ssl/private/server-key.pem
}

FILES:${PN} += "${sysconfdir}"