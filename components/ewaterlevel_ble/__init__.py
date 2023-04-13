from ._shared import (
    EWaterlevel_schema,
    EWaterlevel_to_code,
)

CODEOWNERS = ["@Fabian-Schmidt"]

AUTO_LOAD = ["sensor"]
DEPENDENCIES = ["esp32_ble_tracker"]

CONFIG_SCHEMA = EWaterlevel_schema


async def to_code(config):
    await EWaterlevel_to_code(config)
