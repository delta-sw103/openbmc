#!/bin/sh
 . /usr/local/bin/openbmc-utils.sh
 . /usr/local/fbpackages/utils/ast-functions

KVSET_CMD=/usr/bin/kv

# check the switch IC
Result=$(gpio_get BOARD_ID0)

if [ "$Result" == "1" ]; then
    $KVSET_CMD set "switch_chip" "BRCM"
    # remove unnecessary aggregate sensors
    rm -f /etc/aggregate-sensors.d/0001-vrs.json
else
    $KVSET_CMD set "switch_chip" "MICRO"
fi