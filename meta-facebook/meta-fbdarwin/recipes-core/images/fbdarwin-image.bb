# Copyright 2022-present Facebook. All Rights Reserved.

require recipes-core/images/fboss-lite-image.inc

IMAGE_INSTALL += " \
  serfmon-cache \
  "
IMAGE_INSTALL:remove = "ipmi-lite"
