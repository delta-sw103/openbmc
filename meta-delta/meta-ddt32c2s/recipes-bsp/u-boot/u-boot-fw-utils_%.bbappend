FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://Makefile \
            file://ddt32c2s_defconfig \
            file://ddt32c2s.dts \
            file://ddt32c2s.h \
           "

do_copyfile () {
  cp -v ${WORKDIR}/Makefile ${S}/arch/arm/dts/
  cp -v ${WORKDIR}/ddt32c2s_defconfig ${S}/configs/
  cp -v ${WORKDIR}/ddt32c2s.dts  ${S}/arch/arm/dts/
  cp -v ${WORKDIR}/ddt32c2s.h ${S}/include/configs/
}

do_configure:prepend:mf-mtd-ubifs() {
    #
    # append "ubi.mtd=data0" to bootargs if "mtd-ubifs" is enabled.
    #
    sed -i '/CONFIG_BOOTARGS/ s/"\s*$/ ubi.mtd=data0"/' ${S}/configs/ddt32c2s_defconfig
}

addtask copyfile after do_patch before do_configure
