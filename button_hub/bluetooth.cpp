#include "bluetooth.hpp"

#include <M5Unified.h>
#include <NimBLEDevice.h>

static bool g_device_initialized = false;

namespace bluetooth {

String GetUniqueDeviceName() {
  uint8_t mac_address[6];
  esp_read_mac(mac_address, ESP_MAC_BT);

  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const int charset_size = sizeof(charset) - 1;

  const int i1 = (mac_address[0] + mac_address[1]) % charset_size;
  const int i2 = (mac_address[2] + mac_address[3]) % charset_size;
  const int i3 = (mac_address[4] + mac_address[5]) % charset_size;

  String name = "ハブ ";
  name += charset[i1];
  name += charset[i2];
  name += charset[i3];
  return name;
}

void Init() {
  if (!g_device_initialized) {
    String unique_name = GetUniqueDeviceName();
    NimBLEDevice::init(unique_name.c_str());
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
    NimBLEDevice::setSecurityAuth(true, true, true);
    g_device_initialized = true;
  }
}

}  // namespace bluetooth
