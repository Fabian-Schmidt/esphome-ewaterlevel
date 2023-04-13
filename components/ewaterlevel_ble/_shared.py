import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32_ble_tracker
from esphome.const import (
    CONF_ID,
    CONF_LENGTH,
    CONF_MIN_VALUE,
    CONF_MAX_VALUE,
)

ewaterlevel_ble_ns = cg.esphome_ns.namespace("ewaterlevel_ble")
EWaterlevel = ewaterlevel_ble_ns.class_(
    "EWaterLevel", cg.Component, esp32_ble_tracker.ESPBTDeviceListener
)

EWaterlevel_schema = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(EWaterlevel),
            cv.Optional(CONF_MIN_VALUE, default=10): cv.float_range(min=0, max=250),
            cv.Optional(CONF_MAX_VALUE, default=125): cv.float_range(min=0, max=250),
            cv.Optional(CONF_LENGTH): cv.float_range(min=2, max=38.3)
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
)


async def EWaterlevel_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_ble_device(var, config)

    if CONF_MIN_VALUE in config:
        cg.add(var.set_min_value(config[CONF_MIN_VALUE]))
    if CONF_MAX_VALUE in config:
        cg.add(var.set_max_value(config[CONF_MAX_VALUE]))
    if CONF_LENGTH in config:
        cg.add(var.set_length(config[CONF_LENGTH]))
    
    return var
