#include "bluetooth.hpp"

#include <NimBLEDevice.h>
#include <cstdint>
#include <string>

#include "common.hpp"

static constexpr int kScanDurationSec = 3;

static NimBLEScan* g_ble_scan = nullptr;
static bool g_active = true;

static bool IsIBeacon(NimBLEAdvertisedDevice& device) {
  const auto* mdata =
      reinterpret_cast<const uint8_t*>(device.getManufacturerData().data());
  return device.getManufacturerData().size() >= 25 && mdata[0] == 0x4c &&
         mdata[1] == 0x00 && mdata[2] == 0x02 && mdata[3] == 0x15;
}

class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
 public:
  explicit MyAdvertisedDeviceCallbacks(void (*beacon_callback)(
      const char* name, const uint8_t address[6], const uint8_t uuid[16],
      uint16_t major, uint16_t minor, int8_t tx_power, int rssi))
      : beacon_callback_(beacon_callback) {}

  void onResult(NimBLEAdvertisedDevice* device) override {
    const bool is_ibeacon = IsIBeacon(*device);
    if (is_ibeacon && beacon_callback_) {
      const std::string& manufacturer_data = device->getManufacturerData();
      const auto* uuid =
          reinterpret_cast<const uint8_t*>(manufacturer_data.data() + 4);
      const auto major =
          static_cast<uint16_t>(((manufacturer_data[20] & 0xff) << 8) +
                                (manufacturer_data[21] & 0xff));
      const auto minor =
          static_cast<uint16_t>(((manufacturer_data[22] & 0xff) << 8) +
                                (manufacturer_data[23] & 0xff));
      const int8_t tx_power = manufacturer_data[24];
      beacon_callback_(device->getName().c_str(),
                       device->getAddress().getNative(), uuid, major, minor,
                       tx_power, device->getRSSI());
    }
  }

 private:
  void (*beacon_callback_)(const char* name, const uint8_t address[6],
                           const uint8_t uuid[16], uint16_t major,
                           uint16_t minor, int8_t tx_power, int rssi);
};

static void OnScanComplete(BLEScanResults /* results */) {
  g_ble_scan->clearResults();
  if (g_active) {
    g_ble_scan->start(kScanDurationSec, &OnScanComplete, true);
  }
}

namespace bluetooth {

void Begin(void (*beacon_callback)(const char* name, const uint8_t address[6],
                                   const uint8_t uuid[16], uint16_t major,
                                   uint16_t minor, int8_t tx_power, int rssi)) {
  NimBLEDevice::init("");
  g_ble_scan = NimBLEDevice::getScan();
  g_ble_scan->setAdvertisedDeviceCallbacks(
      new MyAdvertisedDeviceCallbacks(beacon_callback));
  g_ble_scan->setActiveScan(false);
  g_ble_scan->setInterval(100);
  g_ble_scan->setWindow(99);

  g_ble_scan->start(kScanDurationSec, &OnScanComplete, true);
  Serial.println("BLE Scan started");
}

void Stop() {
  g_active = false;
}

void Resume() {
  if (g_active) {
    return;
  }
  g_active = true;
  if (!g_ble_scan->isScanning()) {
    g_ble_scan->start(kScanDurationSec, &OnScanComplete, true);
  }
}

}  // namespace bluetooth
