PV = "${LINUX_VERSION}"

LINUX_VERSION_EXTENSION = "-ddt32c2s"

COMPATIBLE_MACHINE = "ddt32c2s"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
LOCAL_URI += " \
    file://ddt32c2s.cfg \
    file://aspeed-bmc-delta-ddt32c2s.dts \
    file://Makefile \
    "

do_copyfile () {
  cp -v ${S}/Makefile ${S}/arch/arm/boot/dts
  cp -v ${S}/aspeed-bmc-delta-ddt32c2s.dts ${S}/arch/arm/boot/dts/
}

addtask copyfile after do_patch before do_configure

