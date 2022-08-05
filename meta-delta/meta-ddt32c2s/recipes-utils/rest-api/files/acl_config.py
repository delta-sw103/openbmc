from typing import Any, Dict

# type: Dict[str, Any]
RULES = {"/api/sys/server": {"POST": ["wedge400", "wedge400c", "ddt32c2s"]}}

# type: Dict[str, Any]
RULES_REGEXP = {
    # forbid experimental API
    r"^/redfish/v1/Systems/.*": {
        "GET": [],
        "POST": [],
        "DELETE": [],
        "PATCH": [],
        "PUT": [],
    }
}

# example
# RULES = {"/api/sys/bmc": {"GET": ["admin", "test"]}}
# RULES_REGEXP = {r'^/redfish/v1/Systems/server\d+$': {'GET': ['admin','test']}}