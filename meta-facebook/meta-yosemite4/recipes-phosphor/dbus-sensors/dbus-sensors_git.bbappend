FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://0002-PSUSensor-add-ina233-support.patch \
    file://0003-PSUSensor-add-adm1281-support.patch \
    file://0004-PSUSensor-Fix-error-for-decimal-part-of-scalefactor.patch \
"
