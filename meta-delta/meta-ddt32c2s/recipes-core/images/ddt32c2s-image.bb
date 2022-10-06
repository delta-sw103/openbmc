# Copyright 2020-present Delta Electronics, Inc. All Rights Reserved.

require recipes-core/images/fbobmc-image-meta.inc
require ddt32c2s-image-layout.inc
require recipes-core/images/fb-openbmc-image.bb

# Include modules in rootfs
IMAGE_INSTALL += " \
  packagegroup-openbmc-base \
  packagegroup-openbmc-net \
  packagegroup-openbmc-python3 \
  packagegroup-openbmc-rest3 \
  at93cx6-util \
  ast-mdio \
  bitbang \
  cpldupdate \
  cpldupdate-jtag \
  crashdump \
  e2fsprogs \
  fscd \
  fan-util \
  flashrom \
  gbi2ctool \
  gpiocli \
  healthd \
  ipmid \
  ipmitool \
  ipmi-util \
  libcpldupdate-dll-echo \
  libcpldupdate-dll-gpio \
  libcpldupdate-dll-ast-jtag \
  libfruid \
  lldp-util \
  log-util-v2 \
  libncsi \
  ncsi-util \
  ncsid-v2 \
  kcsd \
  openbmc-gpio \
  openbmc-utils \
  psu-util \
  sensor-util \
  sensor-mon \
  threshold-util \
  wedge-eeprom \
  weutil-dhcp-id \
  "


SERIAL_CONSOLES += "115200;ttyS4"
