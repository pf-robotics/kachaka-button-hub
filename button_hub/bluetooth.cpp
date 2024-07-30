#include "bluetooth.hpp"

#include <M5Unified.h>
#include <NimBLEDevice.h>

static bool g_device_initialized = false;

namespace bluetooth {

void Init() {
  if (!g_device_initialized) {
    NimBLEDevice::init("KachakaButtonHub");
    g_device_initialized = true;
  }
}

}  // namespace bluetooth
