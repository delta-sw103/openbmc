
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://fbal.conf \
           "

do_install:append() {
    install -d ${D}${sysconfdir}/sensors.d
    install -m 644 ../fbal.conf ${D}${sysconfdir}/sensors.d/fbal.conf
}
