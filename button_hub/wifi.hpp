#pragma once

#include <WiFi.h>
#include <functional>
#include <vector>

namespace wifi {

// Wi-Fi Client

enum class ConnectState {
  kEmptySsid,
  kTimeout,
  kGiveup,
  kFailed,
  kConnected,
};

ConnectState ConnectToWiFi(const char* ssid, const char* password,
                           int timeout_millis, bool user_interaction_enabled);
ConnectState ConnectToWiFi(const String& ssid, const String& password,
                           const String& ip_address, const String& subnet_mask,
                           const String& gateway, const String& dns_server_1,
                           const String& dns_server_2, int timeout_millis,
                           bool user_interaction_enabled);
String GetIpAddress();

// Wi-Fi AP scan

struct WiFiAp {
  String ssid;
  String bssid;
  int channel;
  bool encryption_type;
};

enum class ScanState {
  kScanning,
  kFailed,
  kSucceeded,
};

void StartApScan();
std::tuple<ScanState, const std::vector<WiFiAp>&> GetScannedWiFiApList();
std::tuple<bool /* scanning */, const std::vector<WiFiAp>&>
GetLatestScannedWiFiApList();

}  // namespace wifi
