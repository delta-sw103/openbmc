#!/usr/bin/env python3
#
# Copyright 2018-present Facebook. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#
import unittest

from common.base_process_running_test import BaseProcessRunningTest
from utils.test_utils import qemu_check


class ProcessRunningTest(BaseProcessRunningTest, unittest.TestCase):
    def set_processes(self):
        self.expected_process = [
            "sshd",
            "healthd",
            "gpiointrd",
            "ntpd",
            "ipmid",
            "ncsid",
            "front-paneld",
            "rest.py",
            "dhcpcd: eth0 [ip4]",
            "dhcpcd: eth0 [ip6]",
            "/usr/local/bin/ipmbd 3 2",
            "/usr/local/bin/ipmbd 1 1",
            "/usr/local/bin/ipmbd 5 3",
            "/usr/local/bin/ipmbd 7 4",
        ]
        if not qemu_check():
            self.expected_process.extend(
                [
                    "fscd",
                    "/usr/local/bin/gpiod slot1 slot2 slot3 slot4",
                    "/usr/local/bin/mTerm_server slot1 /dev/ttyS1",
                    "/usr/local/bin/mTerm_server slot2 /dev/ttyS2",
                    "/usr/local/bin/mTerm_server slot3 /dev/ttyS3",
                    "/usr/local/bin/mTerm_server slot4 /dev/ttyS4",
                ]
            )
