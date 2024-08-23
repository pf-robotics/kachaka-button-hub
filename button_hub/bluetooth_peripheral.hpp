#pragma once

#include <Arduino.h>

namespace bluetooth_peripheral {

bool IsDeviceConnected();

void Begin();
void Stop();

}  // namespace bluetooth_peripheral
