#!/usr/bin/env python
#
# Copyright 2021-present Facebook. All Rights Reserved.
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


from aiohttp.web import Application
from compute_rest_shim import RestShim
from node_bios import (
    get_node_bios,
    get_node_bios_boot_order_trunk,
    get_node_bios_postcode_trunk,
    get_node_bios_plat_info_trunk,
    get_node_bios_boot_mode,
    get_node_bios_clear_cmos,
    get_node_bios_force_boot_setup,
    get_node_bios_boot_order,
)
from node_bmc import get_node_bmc
from node_config import get_node_config
from node_dpb import get_node_dpb
from node_e1s_iocm import get_node_e1s_iocm
from node_enclosure import get_node_enclosure_hdd_status, get_node_enclosure_error
from node_fans import get_node_fans
from node_fruid import get_node_fruid
from node_health import get_node_health
from node_identify import get_node_identify
from node_logs import get_node_logs
from node_mezz import get_node_mezz
from node_scc import get_node_scc
from node_sensors import get_node_sensors
from node_server import get_node_server
from node_uic import get_node_uic
from rest_pal_legacy import pal_get_fru_capability, FRU_CAPABILITY_FRUID_ALL


def populate_e1s_iocm_node(app: Application):
    FRU_E1S_IOCM = 7
    is_supported_iocm = False

    cap = pal_get_fru_capability(FRU_E1S_IOCM)

    if (cap & FRU_CAPABILITY_FRUID_ALL) > 0:
        is_supported_iocm = True

    # Add /api/e1s_iocm end point
    e1s_iocm_shim = RestShim(get_node_e1s_iocm(), "/api/e1s_iocm")
    app.router.add_get(e1s_iocm_shim.path, e1s_iocm_shim.get_handler)

    # Add /api/e1s_iocm/fruid end point
    if is_supported_iocm:
        fruid_shim = RestShim(get_node_fruid("e1s_iocm"), "/api/e1s_iocm/fruid")
        app.router.add_get(fruid_shim.path, fruid_shim.get_handler)
    # /api/e1s_iocm/sensors end point
    # Add /api/e1s_iocm/sensors end point
    sensors_shim = RestShim(get_node_sensors("e1s_iocm"), "/api/e1s_iocm/sensors")
    app.router.add_get(sensors_shim.path, sensors_shim.get_handler)
    # /api/e1s_iocm/logs end point
    # Add /api/e1s_iocm/logs end point
    logs_shim = RestShim(get_node_logs("e1s_iocm"), "/api/e1s_iocm/logs")
    app.router.add_get(logs_shim.path, logs_shim.get_handler)


# Initialize Platform specific Resource Tree
def setup_board_routes(app: Application, write_enabled: bool):

    # Add /api/nic to represent NIC card
    nic_shim = RestShim(get_node_mezz(), "/api/nic")
    app.router.add_get(nic_shim.path, nic_shim.get_handler)

    # /api/nic/fruid
    nic_fruid_shim = RestShim(get_node_fruid("nic"), "/api/nic/fruid")
    app.router.add_get(nic_fruid_shim.path, nic_fruid_shim.get_handler)
    # /api/nic/sensors end point
    nic_sensors_shim = RestShim(get_node_sensors("nic"), "/api/nic/sensors")
    app.router.add_get(nic_sensors_shim.path, nic_sensors_shim.get_handler)
    app.router.add_post(nic_sensors_shim.path, nic_sensors_shim.post_handler)
    # /api/nic/logs end point
    nic_logs_shim = RestShim(get_node_logs("nic"), "/api/nic/logs")
    app.router.add_get(nic_logs_shim.path, nic_logs_shim.get_handler)
    app.router.add_post(nic_logs_shim.path, nic_logs_shim.post_handler)

    # Add /api/uic to represent User Interface Card
    uic_shim = RestShim(get_node_uic(), "/api/uic")
    app.router.add_get(uic_shim.path, uic_shim.get_handler)

    # Add /api/uic/fruid end point
    uic_fruid_shim = RestShim(get_node_fruid("uic"), "/api/uic/fruid")
    app.router.add_get(uic_fruid_shim.path, uic_fruid_shim.get_handler)
    # Add /api/uic/sensors end point
    uic_sensors_shim = RestShim(get_node_sensors("uic"), "/api/uic/sensors")
    app.router.add_get(uic_sensors_shim.path, uic_sensors_shim.get_handler)
    app.router.add_post(uic_sensors_shim.path, uic_sensors_shim.post_handler)
    # Add /api/uic/logs end point
    uic_logs_shim = RestShim(get_node_logs("uic"), "/api/uic/logs")
    app.router.add_get(uic_logs_shim.path, uic_logs_shim.get_handler)
    app.router.add_post(uic_logs_shim.path, uic_logs_shim.post_handler)
    # Add /api/uic/bmc end point
    uic_bmc_shim = RestShim(get_node_bmc(), "/api/uic/bmc")
    app.router.add_get(uic_bmc_shim.path, uic_bmc_shim.get_handler)
    app.router.add_post(uic_bmc_shim.path, uic_bmc_shim.post_handler)
    # Add /api/uic/health end point
    uic_health_shim = RestShim(get_node_health(), "/api/uic/health")
    app.router.add_get(uic_health_shim.path, uic_health_shim.get_handler)
    # Add /api/uic/identify end point
    uic_identify_shim = RestShim(get_node_identify("uic"), "/api/uic/identify")
    app.router.add_get(uic_identify_shim.path, uic_identify_shim.get_handler)
    app.router.add_post(uic_identify_shim.path, uic_identify_shim.post_handler)

    # Add /api/dpb to represent Drive Plane Board
    dpb_shim = RestShim(get_node_dpb(), "/api/dpb")
    app.router.add_get(dpb_shim.path, dpb_shim.get_handler)

    # Add /api/dpb/fruid end point
    dpb_fruid_shim = RestShim(get_node_fruid("dpb"), "/api/dpb/fruid")
    app.router.add_get(dpb_fruid_shim.path, dpb_fruid_shim.get_handler)
    # Add /api/dpb/sensors end point
    dpb_sensors_shim = RestShim(get_node_sensors("dpb"), "/api/dpb/sensors")
    app.router.add_get(dpb_sensors_shim.path, dpb_sensors_shim.get_handler)
    app.router.add_post(dpb_sensors_shim.path, dpb_sensors_shim.post_handler)
    # Add /api/dpb/logs end point
    dpb_logs_shim = RestShim(get_node_logs("dpb"), "/api/dpb/logs")
    app.router.add_get(dpb_logs_shim.path, dpb_logs_shim.get_handler)
    app.router.add_post(dpb_logs_shim.path, dpb_logs_shim.post_handler)

    # Add /api/dpb/fans end point
    dpb_fans_shim = RestShim(get_node_fans(), "/api/dpb/fans")
    app.router.add_get(dpb_fans_shim.path, dpb_fans_shim.get_handler)

    # Add /api/dpb/hdd-status end point
    dpb_hdd_shim = RestShim(get_node_enclosure_hdd_status(), "/api/dpb/hdd-status")
    app.router.add_get(dpb_hdd_shim.path, dpb_hdd_shim.get_handler)
    app.router.add_post(dpb_hdd_shim.path, dpb_hdd_shim.post_handler)

    # Add /api/dpb/error end point
    dpb_error_shim = RestShim(get_node_enclosure_error(), "/api/dpb/error")
    app.router.add_get(dpb_error_shim.path, dpb_error_shim.get_handler)
    # Add /api/scc to represent Storage Controller Card
    scc_shim = RestShim(get_node_scc(), "/api/scc")
    app.router.add_get(scc_shim.path, scc_shim.get_handler)

    # Add /api/scc/fruid end point
    scc_fruid_shim = RestShim(get_node_fruid("scc"), "/api/scc/fruid")
    app.router.add_get(scc_fruid_shim.path, scc_fruid_shim.get_handler)
    # Add /api/scc/sensors end point
    scc_sensors_shim = RestShim(get_node_sensors("scc"), "/api/scc/sensors")
    app.router.add_get(scc_sensors_shim.path, scc_sensors_shim.get_handler)
    app.router.add_post(scc_sensors_shim.path, scc_sensors_shim.post_handler)
    # Add /api/scc/logs end point
    scc_logs_shim = RestShim(get_node_logs("scc"), "/api/scc/logs")
    app.router.add_get(scc_logs_shim.path, scc_logs_shim.get_handler)
    app.router.add_post(scc_logs_shim.path, scc_logs_shim.post_handler)

    # Add /api/e1s_iocm to represent E1.S or IOC Module
    populate_e1s_iocm_node(app)

    # Add /api/server node
    server_shim = RestShim(get_node_server(1), "/api/server")
    app.router.add_get(server_shim.path, server_shim.get_handler)
    app.router.add_post(server_shim.path, server_shim.post_handler)
    # Add /api/server/fruid node
    server_fruid_shim = RestShim(get_node_fruid("server"), "/api/server/fruid")
    app.router.add_get(server_fruid_shim.path, server_fruid_shim.get_handler)
    # Add /api/server/sensors node
    server_sensor_shim = RestShim(get_node_sensors("server"), "/api/server/sensors")
    app.router.add_get(server_sensor_shim.path, server_sensor_shim.get_handler)
    app.router.add_post(server_sensor_shim.path, server_sensor_shim.post_handler)
    # Add /api/server/logs node
    server_logs_shim = RestShim(get_node_logs("server"), "/api/server/logs")
    app.router.add_get(server_logs_shim.path, server_logs_shim.get_handler)
    app.router.add_post(server_logs_shim.path, server_logs_shim.post_handler)
    # Add /api/server/config node
    server_config_shim = RestShim(get_node_config("server"), "/api/server/config")
    app.router.add_get(server_config_shim.path, server_config_shim.get_handler)
    app.router.add_post(server_config_shim.path, server_config_shim.post_handler)
    # Add /api/server/bios node
    server_bios_shim = RestShim(get_node_bios("server"), "/api/server/bios")
    app.router.add_get(server_bios_shim.path, server_bios_shim.get_handler)
    # Add /api/server/bios/boot-order node
    boot_order_trunk_shim = RestShim(
        get_node_bios_boot_order_trunk("server"),
        "/api/server/bios/boot-order",
    )
    app.router.add_get(boot_order_trunk_shim.path, boot_order_trunk_shim.get_handler)
    # Add /api/server/bios/postcode node
    postcode_shim = RestShim(
        get_node_bios_postcode_trunk("server"),
        "/api/server/bios/postcode",
    )
    app.router.add_get(postcode_shim.path, postcode_shim.get_handler)
    # Add /api/server/bios/plat-info node
    plat_info_shim = RestShim(
        get_node_bios_plat_info_trunk("server"),
        "/api/server/bios/plat-info",
    )
    app.router.add_get(plat_info_shim.path, plat_info_shim.get_handler)
    # Add /api/server/bios/boot-order/boot_mode node
    boot_mode_shim = RestShim(
        get_node_bios_boot_mode("server"),
        "/api/server/bios/boot-order/boot_mode",
    )
    app.router.add_get(boot_mode_shim.path, boot_mode_shim.get_handler)
    app.router.add_post(boot_mode_shim.path, boot_mode_shim.post_handler)
    # Add /api/server/bios/boot-order/clear_cmos node
    bios_clear_shim = RestShim(
        get_node_bios_clear_cmos("server"),
        "/api/server/bios/boot-order/clear_cmos",
    )
    app.router.add_get(bios_clear_shim.path, bios_clear_shim.get_handler)
    app.router.add_post(bios_clear_shim.path, bios_clear_shim.post_handler)
    # Add /api/server/bios/boot-order/force_boot_bios_setup node
    force_boot_shim = RestShim(
        get_node_bios_force_boot_setup("server"),
        "/api/server/bios/boot-order/force_boot_bios_setup",
    )
    app.router.add_get(force_boot_shim.path, force_boot_shim.get_handler)
    app.router.add_post(
        force_boot_shim.path,
        force_boot_shim.post_handler,
    )
    # Add /api/server/bios/boot-order/boot_order node
    boot_order_node_shim = RestShim(
        get_node_bios_boot_order("server"),
        "/api/server/bios/boot-order/boot_order",
    )
    app.router.add_get(boot_order_node_shim.path, boot_order_node_shim.get_handler)
    app.router.add_post(boot_order_node_shim.path, boot_order_node_shim.post_handler)
