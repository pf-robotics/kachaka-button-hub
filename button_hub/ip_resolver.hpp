#pragma once

#include <Arduino.h>

namespace ip_resolver {

void Begin();

String GetIpAddressIfPossible(const String& hostname, bool debug_print = false);
String GetCachedIpAddressOrPassthrough(const String& hostname);

}  // namespace ip_resolver
