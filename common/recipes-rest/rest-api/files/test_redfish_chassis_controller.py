import asyncio
import enum
import unittest

import aiohttp.web
import test_mock_modules  # noqa: F401
from aiohttp.test_utils import AioHTTPTestCase, unittest_run_loop
from common_middlewares import jsonerrorhandler


class FakeFruCapability(enum.Enum):
    FRU_CAPABILITY_SERVER = 1
    FRU_CAPABILITY_HAS_DEVICE = 2


class TestChassisService(AioHTTPTestCase):
    async def setUpAsync(self):
        asyncio.set_event_loop(asyncio.new_event_loop())
        power_util_fut1 = asyncio.Future()
        power_util_fut1.set_result((0, "Power status for fru 1 : ON", ""))
        power_util_fut2 = asyncio.Future()
        power_util_fut2.set_result((0, "Power status for fru 1 : ON", ""))
        power_util_fut3 = asyncio.Future()
        power_util_fut3.set_result((0, "Power status for fru 1 : ON", ""))
        power_util_fut4 = asyncio.Future()
        power_util_fut4.set_result((0, "Power status for fru 1 : OFF", ""))
        power_util_side_effects = [
            power_util_fut1,
            power_util_fut2,
            power_util_fut3,
            power_util_fut4,
        ]
        self.patches = [
            unittest.mock.patch(
                "rest_pal_legacy.pal_get_num_slots",
                create=True,
                return_value=4,
            ),
            unittest.mock.patch(
                "pal.pal_get_platform_name",
                create=True,
                return_value="fby2",
            ),
            unittest.mock.patch(
                "pal.pal_fru_name_map",
                create=True,
                return_value={
                    "slot1": 1,
                    "slot2": 2,
                    "slot3": 3,
                    "slot4": 4,
                    "spb": 5,
                    "bmc": 6,
                },
            ),
            unittest.mock.patch(
                "pal.pal_is_fru_prsnt",
                create=True,
                return_value=True,
            ),
            unittest.mock.patch(
                "pal.pal_get_fru_capability",
                new_callable=unittest.mock.MagicMock,
                return_value=[FakeFruCapability.FRU_CAPABILITY_SERVER],
                create=True,
            ),
            unittest.mock.patch(
                "pal.FruCapability", create=True, new=FakeFruCapability
            ),
            unittest.mock.patch(
                "pal.LibPalError",
                create=True,
                new=type("LibPalError", (Exception,), {}),
            ),
            unittest.mock.patch(
                "redfish_chassis_helper.get_fru_info",
                new_callable=unittest.mock.MagicMock,  # python < 3.8 compat
                return_value=asyncio.Future(),
            ),
            unittest.mock.patch(
                "redfish_chassis_helper.get_single_sled_frus",
                new_callable=unittest.mock.MagicMock,  # python < 3.8 compat
                return_value=["bmc", "spb"],
            ),
            unittest.mock.patch(
                "redfish_sensors.get_redfish_sensors_for_server_name",
                new_callable=unittest.mock.MagicMock,  # python < 3.8 compat
                return_value=asyncio.Future(),
            ),
            unittest.mock.patch(
                "aggregate_sensor.aggregate_sensor_init",
                create=True,
                return_value=None,
            ),
            unittest.mock.patch(
                "common_utils.async_exec",
                new_callable=unittest.mock.MagicMock,  # python < 3.8 compat
                side_effect=power_util_side_effects,
            ),
            unittest.mock.patch(
                "os.path.exists",
                new_callable=unittest.mock.MagicMock,  # python < 3.8 compat
                side_effect=[
                    False,
                    True,
                    True,
                    False,
                    True,
                    True,
                    False,
                    True,
                    True,
                    False,
                    True,
                    True,
                    False,
                    True,
                    True,
                ],
            ),
            unittest.mock.patch("kv.kv_get", return_value="on"),
        ]
        for p in self.patches:
            p.start()
            self.addCleanup(p.stop)

        await super().setUpAsync()

    def get_fru_name(self, server_name: str) -> str:
        if server_name == "1":
            return "spb"  # default to assert single sled frus
        else:
            #  if not a single sled fru then return the correct fru_name for assertion
            return server_name.replace("server", "slot")

    @unittest_run_loop
    async def test_get_chassis(self):
        expected_resp = {
            "@odata.context": "/redfish/v1/$metadata#ChassisCollection.ChassisCollection",  # noqa: B950
            "@odata.id": "/redfish/v1/Chassis",
            "@odata.type": "#ChassisCollection.ChassisCollection",
            "Name": "Chassis Collection",
            "Members@odata.count": 5,
            "Members": [
                {"@odata.id": "/redfish/v1/Chassis/1"},
                {"@odata.id": "/redfish/v1/Chassis/server1"},
                {"@odata.id": "/redfish/v1/Chassis/server2"},
                {"@odata.id": "/redfish/v1/Chassis/server3"},
                {"@odata.id": "/redfish/v1/Chassis/server4"},
            ],
        }
        req = await self.client.request("GET", "/redfish/v1/Chassis")
        resp = await req.json()
        self.maxDiff = None
        self.assertEqual(resp, expected_resp)
        self.assertEqual(req.status, 200)

    @unittest_run_loop
    async def test_get_chassis_with_accelerators(self):
        import pal

        pal.pal_fru_name_map.return_value = {
            "slot1": 1,
            "slot2": 2,
            "slot3": 3,
            "slot4": 4,
            "slot1-2U-exp": 5,
            "slot1-2U-top": 6,
            "slot1-2U-bot": 7,
        }
        pal.pal_get_fru_capability.side_effect = [
            [FakeFruCapability.FRU_CAPABILITY_SERVER],
            [],
            [FakeFruCapability.FRU_CAPABILITY_HAS_DEVICE],
            [FakeFruCapability.FRU_CAPABILITY_HAS_DEVICE],
        ]
        pal.pal_is_fru_prsnt.side_effect = [True, False, False, False, True, True, True]
        expected_resp = {
            "@odata.context": "/redfish/v1/$metadata#ChassisCollection.ChassisCollection",  # noqa: B950
            "@odata.id": "/redfish/v1/Chassis",
            "@odata.type": "#ChassisCollection.ChassisCollection",
            "Name": "Chassis Collection",
            "Members@odata.count": 4,
            "Members": [
                {"@odata.id": "/redfish/v1/Chassis/1"},
                {"@odata.id": "/redfish/v1/Chassis/server1"},
                {"@odata.id": "/redfish/v1/Chassis/accelerator0"},
                {"@odata.id": "/redfish/v1/Chassis/accelerator1"},
            ],
        }
        req = await self.client.request("GET", "/redfish/v1/Chassis")
        resp = await req.json()
        self.maxDiff = None
        self.assertEqual(resp, expected_resp)
        self.assertEqual(req.status, 200)

    @unittest_run_loop
    async def test_get_chassis_members(self):
        import redfish_chassis_helper

        "Testing chassis members for both single sled frus and multisled frus"
        for server_name in ["1", "server1", "server2", "server3", "server4"]:
            fru_name = self.get_fru_name(server_name)
            with self.subTest(server_name=server_name):
                redfish_chassis_helper.get_fru_info.return_value = asyncio.Future()
                redfish_chassis_helper.get_fru_info.return_value.set_result(
                    redfish_chassis_helper.FruInfo(
                        fru_name, "Wiwynn", "WTL19121DSMA1", "Yosemite V2 MP"
                    )
                )
                expected_power = "On"
                if server_name == "1":
                    expected_name = "Computer System Chassis"
                else:
                    expected_name = fru_name
                if server_name == "server4":
                    expected_power = "Off"

                expected_resp = {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/{}".format(server_name),
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": expected_name,
                    "ChassisType": "RackMount",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "PowerState": expected_power,
                    "IndicatorLED": "Lit",
                    "Sensors": {
                        "@odata.id": "/redfish/v1/Chassis/{}/Sensors".format(
                            server_name
                        )
                    },
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                }
                req = await self.client.request(
                    "GET", "/redfish/v1/Chassis/{}".format(server_name)
                )
                resp = await req.json()
                self.maxDiff = None
                self.assertEqual(resp, expected_resp)
                self.assertEqual(req.status, 200)

    @unittest_run_loop
    async def test_multislot_routes_return_notfound_on_singleslot(self):
        with unittest.mock.patch(
            "rest_pal_legacy.pal_get_num_slots",
            create=True,
            return_value=1,
        ):
            req = await self.client.request("GET", "/redfish/v1/Chassis/server4")
            self.assertEqual(req.status, 404)

    @unittest_run_loop
    async def test_get_chassis_expand_expands_children(self):
        import redfish_chassis_helper

        redfish_chassis_helper.get_fru_info.return_value = asyncio.Future()
        redfish_chassis_helper.get_fru_info.return_value.set_result(
            redfish_chassis_helper.FruInfo(
                "x", "Wiwynn", "WTL19121DSMA1", "Yosemite V2 MP"
            )
        )
        expected_resp = {
            "@odata.context": "/redfish/v1/$metadata#ChassisCollection.ChassisCollection",
            "@odata.id": "/redfish/v1/Chassis",
            "@odata.type": "#ChassisCollection.ChassisCollection",
            "Name": "Chassis Collection",
            "Members@odata.count": 5,
            "Members": [
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/1",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "Computer System Chassis",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/1/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server1",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot1",
                    "IndicatorLED": "Lit",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/server1/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server2",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot2",
                    "IndicatorLED": "Lit",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/server2/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server3",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot3",
                    "IndicatorLED": "Lit",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/server3/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server4",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot4",
                    "IndicatorLED": "Lit",
                    "ChassisType": "RackMount",
                    "PowerState": "Off",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/server4/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
            ],
        }
        req = await self.client.request("GET", "/redfish/v1/Chassis?$expand=1")
        resp = await req.json()
        self.maxDiff = None
        self.assertEqual(resp, expected_resp)
        self.assertEqual(req.status, 200)

    @unittest_run_loop
    async def test_get_chassis_expand__level_1_expands_children(self):
        import redfish_chassis_helper

        redfish_chassis_helper.get_fru_info.return_value = asyncio.Future()
        redfish_chassis_helper.get_fru_info.return_value.set_result(
            redfish_chassis_helper.FruInfo(
                "x", "Wiwynn", "WTL19121DSMA1", "Yosemite V2 MP"
            )
        )
        expected_resp = {
            "@odata.context": "/redfish/v1/$metadata#ChassisCollection.ChassisCollection",
            "@odata.id": "/redfish/v1/Chassis",
            "@odata.type": "#ChassisCollection.ChassisCollection",
            "Name": "Chassis Collection",
            "Members@odata.count": 5,
            "Members": [
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/1",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "Computer System Chassis",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/1/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server1",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot1",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/server1/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server2",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot2",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/server2/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server3",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot3",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/server3/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server4",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot4",
                    "ChassisType": "RackMount",
                    "PowerState": "Off",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": {"@odata.id": "/redfish/v1/Chassis/server4/Sensors"},
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
            ],
        }
        req = await self.client.request(
            "GET", "/redfish/v1/Chassis?$expand=.($levels=1)"
        )
        resp = await req.json()
        self.maxDiff = None
        self.assertEqual(resp, expected_resp)
        self.assertEqual(req.status, 200)

    @unittest_run_loop
    async def test_get_chassis_expand_invalid_param_returns_400(self):
        req = await self.client.request("GET", "/redfish/v1/Chassis?$expand=DERP")
        resp = await req.json()
        self.assertEqual(resp, {"reason": "Invalid expand level supplied: DERP"})
        self.assertEqual(req.status, 400)

    @unittest_run_loop
    async def test_get_chassis_expand_asterisk_expands_all_children(self):
        import redfish_chassis_helper
        import redfish_sensors

        redfish_chassis_helper.get_fru_info.return_value = asyncio.Future()
        redfish_chassis_helper.get_fru_info.return_value.set_result(
            redfish_chassis_helper.FruInfo(
                "x", "Wiwynn", "WTL19121DSMA1", "Yosemite V2 MP"
            )
        )
        sensor_resp = {
            "@odata.type": "#SensorCollection.SensorCollection",
            "Name": "Chassis sensors",
            "Members@odata.count": 41,
            "Members": [
                {"@odata.id": "/redfish/v1/Chassis/1/Sensors/bmc_224"},
            ],
            "@odata.id": "/redfish/v1/Chassis/1/Sensors",
        }

        redfish_sensors.get_redfish_sensors_for_server_name.return_value = (
            asyncio.Future()
        )
        redfish_sensors.get_redfish_sensors_for_server_name.return_value.set_result(
            sensor_resp
        )
        expected_resp = {
            "@odata.context": "/redfish/v1/$metadata#ChassisCollection.ChassisCollection",
            "@odata.id": "/redfish/v1/Chassis",
            "@odata.type": "#ChassisCollection.ChassisCollection",
            "Name": "Chassis Collection",
            "Members@odata.count": 5,
            "Members": [
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/1",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "Computer System Chassis",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": sensor_resp,
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server1",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot1",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": sensor_resp,
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server2",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot2",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": sensor_resp,
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server3",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot3",
                    "ChassisType": "RackMount",
                    "PowerState": "On",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": sensor_resp,
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
                {
                    "@odata.context": "/redfish/v1/$metadata#Chassis.Chassis",
                    "@odata.id": "/redfish/v1/Chassis/server4",
                    "@odata.type": "#Chassis.v1_15_0.Chassis",
                    "Id": "1",
                    "Name": "slot4",
                    "ChassisType": "RackMount",
                    "PowerState": "Off",
                    "IndicatorLED": "Lit",
                    "Manufacturer": "Wiwynn",
                    "Model": "Yosemite V2 MP",
                    "SerialNumber": "WTL19121DSMA1",
                    "Status": {"State": "Enabled", "Health": "OK"},
                    "Sensors": sensor_resp,
                    "Links": {"ManagedBy": [{"@odata.id": "/redfish/v1/Managers/1"}]},
                },
            ],
        }
        req = await self.client.request("GET", "/redfish/v1/Chassis?$expand=*")
        resp = await req.json()
        self.maxDiff = None
        self.assertEqual(resp, expected_resp)
        self.assertEqual(req.status, 200)

    @unittest_run_loop
    async def test_redfish_change_fpc_led_state_returns_400_on_invalid_desired_state(
        self,
    ):
        req = await self.client.request(
            "PATCH", "/redfish/v1/Chassis/server1", json={"IndicatorLED": "Derp"}
        )
        resp = await req.json()
        self.maxDiff = None
        self.assertEqual(
            resp,
            {
                "reason": "Derp is invalid IndicatorLED state, valid states are: Lit, Off",
                "status": "Bad Request",
            },
        )
        self.assertEqual(req.status, 400)

    @unittest_run_loop
    async def test_redfish_change_fpc_led_state_returns_200_on_successful_led_status_change(
        self,
    ):
        async_exec_fut = asyncio.Future()
        async_exec_fut.set_result((0, "", ""))
        with unittest.mock.patch(
            "os.path.exists", return_value=True
        ), unittest.mock.patch(
            "common_utils.async_exec", return_value=async_exec_fut
        ) as ae_mock:
            req = await self.client.request(
                "PATCH", "/redfish/v1/Chassis/server1", json={"IndicatorLED": "Lit"}
            )
            resp = await req.json()
            self.maxDiff = None
            self.assertEqual(
                resp,
                {
                    "success": True,
                },
            )
            self.assertEqual(req.status, 200)
            ae_mock.assert_called_once_with(
                ["/usr/bin/fpc-util", "slot1", "--identify", "on"]
            )

    async def get_application(self):
        webapp = aiohttp.web.Application(middlewares=[jsonerrorhandler])
        from redfish_common_routes import Redfish

        redfish = Redfish()
        redfish.setup_redfish_common_routes(webapp)
        return webapp
