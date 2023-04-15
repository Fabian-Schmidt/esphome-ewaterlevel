#pragma once

#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP32

namespace esphome {
namespace ewaterlevel_ble {

struct ewaterlevel_data {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  // 0x02010617FF
  u_int8_t preamble[5];
  // "WTRL" - Waterlevel
  char header[4];
  u_int16_t counter;
  /*
   * 00 - Invalid value / Too Low
   * 01 - Valid value / Very Low
   * 02 - Valid value / Low
   * 03 - Valid value / Medium
   * 04 - Valid value / High
   * 05 - Valid value / Very High
   * 06.. - Invalid value / Too High
   */
  u_int8_t state_a;
  u_int16_t unknown_a;
  u_int16_t battery_voltage;
  u_int16_t value;
  u_int8_t version_sw_low;
  u_int8_t version_sw_high;
  u_int8_t version_hw_low;
  u_int8_t version_hw_high;
  /*
   * Bits:
   * 0 -
   * 1 -
   * 2 - ?Water detected?
   * 3 - ??
   * 4 - Always set
   * 5 - ??
   * 6 - ??
   * 7 - ??
   */
  u_int8_t state_b;
  // Repeat of upper bit of `value`.
  u_int8_t value_high;
  u_int16_t short_pin_length;
  u_int16_t long_pin_length;
  /*
   * Bits:
   * 0 - Fast mode
   * 1 - Init mode
   * 2 - ??
   * 3 - ??
   * 4 - ??
   * 5 - Value increasing
   * 6 - Value decreasing
   * 7 - ??
   */
  u_int8_t state_c;
  u_int8_t empty;

  inline bool validate_header() const {
    return this->preamble[0] == 0x02 && this->preamble[1] == 0x01 && this->preamble[2] == 0x06 &&
           this->preamble[3] == 0x17 && this->preamble[4] == 0xFF && this->header[0] == 'W' && this->header[1] == 'T' &&
           this->header[2] == 'R' && this->header[3] == 'L';
  }

  inline bool validate_state_a() const { return this->state_a > 0x00 && this->state_a < 0x06; }

  inline float read_counter() const { return 0.001f * convert_big_endian(this->counter) * 4.0f; }

  inline float read_battery_voltage() const { return 0.001f * this->battery_voltage; }

  inline float read_value() const { return 0.01f * 0.5f * this->value; }

  inline float read_short_pin_length() const { return 0.1f * this->short_pin_length; }

  inline float read_long_pin_length() const { return 0.1f * this->long_pin_length; }

  inline const char *read_state_a() const {
    switch (this->state_a) {
      case 0x00:
        return "Too Low";
      case 0x01:
        return "Very Low";
      case 0x02:
        return "Low";
      case 0x03:
        return "Normal";
      case 0x04:
        return "High";
      case 0x05:
        return "Very High";
      default:
        return "Too High";
    }
  }

} __attribute__((packed));

static inline std::string format_ble_address_pretty(uint64_t address) {
  u_int8_t *mac = (uint8_t *) &address;
  return str_snprintf("%02X:%02X:%02X:%02X:%02X:%02X", 17, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static inline float clamp_percentage(const float percent) {
  if (percent < 0.0f) {
    return 0.0f;
  }
  if (percent > 100.0f) {
    return 100.0f;
  }
  return percent;
}

class EWaterLevel : public Component, public esp32_ble_tracker::ESPBTDeviceListener {
 public:
  void dump_config() override;

  void set_address(uint64_t val) { this->address_ = val; };
  void set_min_value(float val) { this->min_value_ = val; };
  void set_max_value(float val) { this->max_value_ = val; };
  void set_length(float val) { this->length_ = val; };
  void set_time(sensor::Sensor *val) { this->time_ = val; };
  void set_value(sensor::Sensor *val) { this->value_ = val; };
  void set_height(sensor::Sensor *val) { this->height_ = val; };
  void set_level(sensor::Sensor *val) { this->level_ = val; };
  void set_battery_voltage(sensor::Sensor *val) { this->battery_voltage_ = val; };
  void set_battery_level(sensor::Sensor *val) { this->battery_level_ = val; };

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;

 protected:
  uint64_t address_{0};
  float min_value_{NAN};
  float max_value_{NAN};
  // Below 2cm no accurate measurement is possible.
  constexpr static float min_length_{2.0};
  float length_{NAN};
  sensor::Sensor *time_{nullptr};
  sensor::Sensor *value_{nullptr};
  sensor::Sensor *height_{nullptr};
  sensor::Sensor *level_{nullptr};
  sensor::Sensor *battery_voltage_{nullptr};
  sensor::Sensor *battery_level_{nullptr};

  float water_height_in_cm_(const ewaterlevel_data *data);

  float inline pin_length_(const ewaterlevel_data *data) {
    if (!std::isnan(this->length_) && this->length_ > 0.0f) {
      return this->length_;
    } else {
      return data->read_long_pin_length();
    }
  };
};

}  // namespace ewaterlevel_ble
}  // namespace esphome

#endif
