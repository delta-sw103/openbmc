from typing import Any, Dict
# type: Dict[str, Any]

RULES = {"/api/sys/server": {"POST": ["ddt32c2s"]}}

RULES_REGEXP = {
    # forbid experimental API
    r"^/redfish/v1/Managers/.*": {
        "GET": [],
        "POST": [],
    },
    r"^/redfish/v1/Systems/.*": {
        "GET": [],
        "POST": ["root"],
        "DELETE": [],
        "PATCH": [],
        "PUT": [],
    }
}

# example
# RULES = {"/api/sys/bmc": {"GET": ["admin", "test"]}}
# RULES_REGEXP = {r'^/redfish/v1/Systems/server\d+$': {'GET': ['admin','test']}}