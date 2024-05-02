#pragma once

#include <WiFi.h>
#include <functional>

bool ConnectToWiFi(const char* ssid, const char* password,
                   const std::function<bool()>& giveup);
bool IsWiFiConnected();
String GetIPAddress();
