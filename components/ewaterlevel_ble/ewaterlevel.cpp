#include "ewaterlevel.h"

#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP32

namespace esphome {
namespace ewaterlevel_ble {

static const char *const TAG = "ewaterlevel_ble";

void EWaterLevel::dump_config() {
  ESP_LOGCONFIG(TAG, "E-Waterlevel");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", format_hex(&this->address_, 6).c_str());
  ESP_LOGCONFIG(TAG, "  Min value: %.3f", this->min_value_);
  ESP_LOGCONFIG(TAG, "  Max value: %.3f", this->max_value_);
  if (!std::isnan(this->length_) && this->length_ > 0.0f) {
    ESP_LOGCONFIG(TAG, "  Length: %.1fcm", this->length_);
  }
  LOG_SENSOR("  ", "Time", this->time_);
  LOG_SENSOR("  ", "Value", this->value_);
  LOG_SENSOR("  ", "Height", this->height_);
  LOG_SENSOR("  ", "Level", this->level_);
  LOG_SENSOR("  ", "Battery Level", this->battery_level_);
  LOG_SENSOR("  ", "Battery Voltage", this->battery_voltage_);
}

/**
 * Parse all incoming BLE payloads to see if it is a matching BLE advertisement.
 * Currently this supports the following products:
 *
 * - E-Trailer E-Waterlevel
 *
 * The following points are used to identify a sensor:
 *
 * - Bluetooth data frame size
 * - Bluetooth data frame header
 */
bool EWaterLevel::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (this->address_ != 0) {
    if (device.address_uint64() != this->address_) {
      return false;
    }
    ESP_LOGVV(TAG, "parse_device(): MAC address %s found.", device.address_str().c_str());
  }

  const esp_ble_gap_cb_param_t::ble_scan_result_evt_param &param = device.get_scan_result();
  const uint8_t *payload = param.ble_adv;
  uint8_t len = param.adv_data_len + param.scan_rsp_len;
  if (len == sizeof(ewaterlevel_data)) {
    const ewaterlevel_data *data = (ewaterlevel_data *) payload;
    if (!data->validate_header()) {
      return false;
    }

    if (this->address_ == 0) {
      ESP_LOGI(TAG, "E-Waterlevel SENSOR FOUND: %s", device.address_str().c_str());
    }

    ESP_LOGV(TAG, "[%s] Sensor data: %s", device.address_str().c_str(),
             format_hex_pretty(param.ble_adv + 9, param.adv_data_len + param.scan_rsp_len - 10).c_str());
    ESP_LOGD(TAG, "[%s] HW: V%u.%u SW: V%u.%u, ShortPin: %.1fcm, LongPin: %.1fcm", device.address_str().c_str(),
             data->version_hw_high, data->version_hw_low, data->version_sw_high, data->version_sw_low,
             data->read_short_pin_length(), data->read_long_pin_length());
    ESP_LOGD(TAG, "[%s] State_A: %s, State_B: %s, State_C: %s", device.address_str().c_str(),
             format_hex(&data->state_a, 1).c_str(), format_hex(&data->state_b, 1).c_str(),
             format_hex(&data->state_c, 1).c_str());
    ESP_LOGI(TAG, "[%s] Time: %.2f, Bat: %.3fV, Value: %.3f", device.address_str().c_str(), data->read_counter(),
             data->read_battery_voltage(), data->read_value());
    ESP_LOGI(TAG, "[%s] Waterlevel: %.1fcm, Percentage: %.1f%%", device.address_str().c_str(),
             this->water_height_in_cm_(data),
             clamp_percentage(this->water_height_in_cm_(data) / this->pin_length_(data) * 100.0f));

    if (this->time_ != nullptr) {
      this->time_->publish_state(data->read_counter());
    }

    if (this->value_ != nullptr) {
      this->value_->publish_state(data->read_value());
    }

    if (this->height_ != nullptr) {
      this->height_->publish_state(this->water_height_in_cm_(data));
    }

    if (this->level_ != nullptr) {
      const auto height_in_cm = this->water_height_in_cm_(data);
      const auto pin_length = this->pin_length_(data);
      this->level_->publish_state(clamp_percentage(height_in_cm / pin_length * 100.0f));
    }

    if (this->battery_voltage_ != nullptr) {
      this->battery_voltage_->publish_state(data->read_battery_voltage());
    }

    if (this->battery_level_ != nullptr) {
      const auto battery_volt = data->read_battery_voltage();
      if (std::isnan(battery_volt)) {
        this->battery_level_->publish_state(NAN);
      } else {
        float percent = (battery_volt - 2.2f) / 0.65f * 100.0f;
        this->battery_level_->publish_state(clamp_percentage(percent));
      }
    }

    return true;
  }
  return false;
}

float EWaterLevel::water_height_in_cm_(const ewaterlevel_data *data) {
  if (!data->validate_state_a()) {
    return 0.0f;
  }

  const auto value = data->read_value();
  const auto pin_length = this->pin_length_(data);
  const auto scaling_factor = (this->max_value_ - this->min_value_) / (pin_length - this->min_length_);
  const auto height = (value - this->min_value_) / scaling_factor + this->min_length_;

  if (height < 0.0f) {
    return 0.0f;
  } else if (height > pin_length) {
    return pin_length;
  } else {
    return height;
  }
}

}  // namespace ewaterlevel_ble
}  // namespace esphome

#endif
