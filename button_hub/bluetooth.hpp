#pragma once

#include <Arduino.h>
#include <TickTwo.h>
#include <cstdint>
#include <functional>

namespace bluetooth {

void Begin(void (*beacon_callback)(const char* name, const uint8_t address[6],
                                   const uint8_t uuid[16], uint16_t major,
                                   uint16_t minor, int8_t tx_power, int rssi));
void Stop();
void Resume();

class SetupTicker {
 public:
  explicit SetupTicker(
      const int interval_ms, std::function<bool()> condition_to_start_bluetooth,
      void (*beacon_callback)(const char* name, const uint8_t address[6],
                              const uint8_t uuid[16], uint16_t major,
                              uint16_t minor, int8_t tx_power, int rssi))
      : condition_(std::move(condition_to_start_bluetooth)),
        ticker_{[this]() { Callback(); }, interval_ms},
        beacon_callback_(beacon_callback) {}

  SetupTicker(const SetupTicker&) = delete;
  SetupTicker& operator=(const SetupTicker&) = delete;

  void Start() { ticker_.start(); }
  void Update() { ticker_.update(); }

 private:
  void Callback() {
    if (condition_()) {
      Begin(beacon_callback_);
      ticker_.stop();
    }
  }

  std::function<bool()> condition_;
  TickTwo ticker_;
  void (*beacon_callback_)(const char* name, const uint8_t address[6],
                           const uint8_t uuid[16], uint16_t major,
                           uint16_t minor, int8_t tx_power, int rssi);
};

}  // namespace bluetooth
