
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_BATTERY_LEVEL,
    CONF_BATTERY_VOLTAGE,
    CONF_MAC_ADDRESS,
    CONF_LEVEL,
    CONF_TIME,
    CONF_HEIGHT,
    CONF_VALUE,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_VOLTAGE,
    ICON_RULER,
    ICON_WATER_PERCENT,
    STATE_CLASS_MEASUREMENT,
    UNIT_CENTIMETER,
    UNIT_PERCENT,
    UNIT_SECOND,
    UNIT_VOLT,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from ._shared import (
    EWaterlevel_schema,
    EWaterlevel_to_code,
)

DEPENDENCIES = ["esp32_ble_tracker"]

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
            cv.Optional(CONF_TIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_VALUE): sensor.sensor_schema(
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_HEIGHT): sensor.sensor_schema(
                unit_of_measurement=UNIT_CENTIMETER,
                icon=ICON_RULER,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_DISTANCE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_WATER_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(EWaterlevel_schema)
)


async def to_code(config):
    var = await EWaterlevel_to_code(config)
    cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))

    if CONF_TIME in config:
        sens = await sensor.new_sensor(config[CONF_TIME])
        cg.add(var.set_time(sens))
    if CONF_VALUE in config:
        sens = await sensor.new_sensor(config[CONF_VALUE])
        cg.add(var.set_value(sens))
    if CONF_HEIGHT in config:
        sens = await sensor.new_sensor(config[CONF_HEIGHT])
        cg.add(var.set_height(sens))
    if CONF_LEVEL in config:
        sens = await sensor.new_sensor(config[CONF_LEVEL])
        cg.add(var.set_level(sens))
    if CONF_BATTERY_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_VOLTAGE])
        cg.add(var.set_battery_voltage(sens))
    if CONF_BATTERY_LEVEL in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_LEVEL])
        cg.add(var.set_battery_level(sens))
