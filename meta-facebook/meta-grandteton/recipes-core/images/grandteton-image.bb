# Copyright 2020-present Facebook. All Rights Reserved.

require recipes-core/images/fbobmc-image-meta.inc
require grandteton-image-layout.inc
require recipes-core/images/fb-zstd-rootfs.inc

require recipes-core/images/fb-openbmc-image.bb

PROVIDES += "inspirationpoint-image"

# Include modules in rootfs
IMAGE_INSTALL += " \
  packagegroup-openbmc-base \
  packagegroup-openbmc-net \
  packagegroup-openbmc-python3 \
  packagegroup-openbmc-rest3 \
  plat-utils \
  openbmc-utils \
  fruid \
  ipmid \
  ipmitool \
  ipmi-util \
  ipmbd \
  ipmb-util \
  kcsd \
  sensor-util \
  sensor-mon \
  power-util \
  peci-util-v2 \
  pciutils \
  me-util \
  log-util-v2 \
  guid-util \
  mctp-util \
  mterm \
  healthd \
  cfg-util \
  fan-util \
  hgxmgr \
  gpiod \
  asd \
  asd-test \
  bios-util \
  pldmd \
  front-paneld \
  mctpd \
  ncsi-util \
  crashdump \
  attest-util \
"
