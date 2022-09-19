inherit systemd

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

LOCAL_URI += " \
    file://fetch-backports.service \
    file://setup_i2c.service \
    file://power-on.service \
    file://setup_board.service \
    file://enable_watchdog_ext_signal.service \
    file://mount_data1.service \
    "

FILES:${PN} += "${systemd_system_unitdir} /usr/local/bin"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -d ${D}/usr/local/bin
    install -m 0644 fetch-backports.service ${D}${systemd_system_unitdir}
    install -m 0644 setup_i2c.service ${D}${systemd_system_unitdir}
    install -m 0644 setup_board.service ${D}${systemd_system_unitdir}
    install -m 0644 power-on.service ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/enable_watchdog_ext_signal.service ${D}${systemd_system_unitdir}
}


SYSTEMD_SERVICE:${PN} += "power-on.service \
                      setup_board.service \
                      enable_watchdog_ext_signal.service \
                      setup_i2c.service \
                      fetch-backports.service \
"
