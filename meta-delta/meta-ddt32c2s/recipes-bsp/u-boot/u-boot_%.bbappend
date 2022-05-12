FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://Makefile \
            file://ddt32c2s.dts \
            file://ddt32c2s_defconfig.append \
            file://ddt32c2s.h \
           "

do_copyfile () {
  cp -v ${WORKDIR}/Makefile ${S}/arch/arm/dts/
  cp -v ${WORKDIR}/ddt32c2s.dts  ${S}/arch/arm/dts/
  cp -v ${WORKDIR}/ddt32c2s.h ${S}/include/configs/
}

addtask copyfile after do_patch before do_configure
