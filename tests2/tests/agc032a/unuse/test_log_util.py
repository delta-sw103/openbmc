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

from common.base_log_util_test import BaseLogUtilTest


class AllLogUtilTest(BaseLogUtilTest, unittest.TestCase):
    FRU = "all"

    @unittest.skip("not available")
    def test_log_clear(self):
        pass


class Fan1LogUtilTest(AllLogUtilTest):
    FRU = "fan1"


class Fan2LogUtilTest(AllLogUtilTest):
    FRU = "fan2"


class Fan3LogUtilTest(AllLogUtilTest):
    FRU = "fan3"


class Fan4LogUtilTest(AllLogUtilTest):
    FRU = "fan4"


class Fan5LogUtilTest(AllLogUtilTest):
    FRU = "fan5"


class Fan6LogUtilTest(AllLogUtilTest):
    FRU = "fan6"


class SysLogUtilTest(BaseLogUtilTest, unittest.TestCase):
    FRU = "sys"

class Psu1LogUilTest(BaseLogUtilTest, unittest.TestCase):
    FRU = "psu1"
class SmbLogUilTest(BaseLogUtilTest, unittest.TestCase):
    FRU = "smb"
class Pus2LogUilTest(BaseLogUtilTest, unittest.TestCase):
    FRU = "psu2"
