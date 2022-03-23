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

from common.base_fans_test import CommonFanUtilBasedFansTest
from utils.cit_logger import Logger
from utils.test_utils import qemu_check
from utils.shell_util import run_shell_cmd

class FansTest(CommonFanUtilBasedFansTest, unittest.TestCase):
    def setUp(self):
        Logger.start(name=self._testMethodName)
        self.fans = [0, 1,2,3,4,5,6,7,8,9,10,11]
        self.pwms = {0: [0], 1: [1],2: [2],3: [3],4: [4],5: [5],6: [6],7: [7],8: [8],9: [9],10: [10],11: [11]}
        self.names = {"Fan 0": 0, "Fan 1": 1,"Fan 2": 2, "Fan 3": 3,"Fan 4": 4, "Fan 5": 5,"Fan 6": 6, "Fan 7": 7,"Fan 8": 8, "Fan 9": 9,"Fan 10": 10, "Fan 11": 11}
        self.start_fan_ctrl_cmd = ["/usr/bin/sv start fscd"]
        self.kill_fan_ctrl_cmd = ["/usr/bin/sv stop fscd"]
        self.start_fan_ctrl_cmd = ["/usr/bin/sv start fscd"]

    def tearDown(self):
        Logger.info("Finished logging for {}".format(self._testMethodName))
        self.kill_fan_controller()
        self.start_fan_controller()

    def test_fan_pwm_set(self):
        pass
