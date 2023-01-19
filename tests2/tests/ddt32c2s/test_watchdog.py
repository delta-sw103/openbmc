#!/usr/bin/env python3
#
# Copyright 2020-present Facebook. All Rights Reserved.
#
# This software may be used and distributed according to the terms of the
# GNU General Public License version 2.
#
import unittest

from common.base_watchdog_test import WatchdogTest
from utils.test_utils import qemu_check

@unittest.skipIf(qemu_check(), "test env is QEMU, skipped")
class WatchdogTest(WatchdogTest, unittest.TestCase):
    def set_kill_watchdog_daemon_cmd(self):
        self.kill_watchdog_daemon_cmd = ["/usr/bin/sv stop fscd"]

    def set_start_watchdog_daemon_cmd(self):
        self.start_watchdog_daemon_cmd = ["/usr/bin/sv start fscd"]
