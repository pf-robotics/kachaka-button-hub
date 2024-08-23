#include "wifi.hpp"

#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiAP.h>

#include "screen.hpp"

namespace wifi {

static constexpr int kWiFiButtonVisibilityTimeout = 3 * 1000;

ConnectState ConnectToWiFi(const char* ssid, const char* password,
                           int timeout_millis, bool user_interaction_enabled) {
  if (strlen(ssid) == 0) {
    return ConnectState::kEmptySsid;
  }

  Serial.print("Connecting to Wi-Fi network: ");
  bool initial_screen = true;
  if (user_interaction_enabled) {
    screen::DrawWiFiConnectingPage(true);
  }

  // Choose the best WiFi AP according to signal strength
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);

  WiFi.setMinSecurity(WIFI_AUTH_WEP);

  const auto now = millis();
  WiFi.begin(ssid, password);
  do {
    if (user_interaction_enabled) {
      if (initial_screen && millis() - now > kWiFiButtonVisibilityTimeout) {
        initial_screen = false;
        screen::DrawWiFiConnectingPage(false);
      }
      M5.update();
      bool giveup = M5.BtnA.isPressed();
      if (giveup) {
        Serial.println("Giving up connection");
        WiFi.disconnect();
        return ConnectState::kGiveup;
      }
    }
    if (millis() - now > timeout_millis) {
      Serial.println("Connection timeout");
      WiFi.disconnect();
      return ConnectState::kTimeout;
    }
    vTaskDelay(1);
  } while (WiFi.status() != WL_CONNECTED);
  Serial.printf("Connected: %s\n", GetIpAddress().c_str());
  return ConnectState::kConnected;
}

String GetIpAddress() {
  return WiFi.localIP().toString();
}

static std::vector<WiFiAp> wifi_ap_list_ = {};

void StartApScan() {
  Serial.println("Start scanning...");
  WiFi.scanNetworks(/* async= */ false);
}

std::tuple<ScanState, const std::vector<WiFiAp>&> GetScannedWiFiApList() {
  const int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_FAILED) {
    return {ScanState::kFailed, {}};
  }
  if (n == WIFI_SCAN_RUNNING || n <= 0) {
    return {ScanState::kScanning, {}};
  }

  std::vector<WiFiAp> ap_list;
  for (int i = 0; i < n; ++i) {
    ap_list.push_back({WiFi.SSID(i), WiFi.BSSIDstr(i), WiFi.channel(i),
                       WiFi.encryptionType(i)});
  }
  WiFi.scanDelete();
  wifi_ap_list_ = std::move(ap_list);
  return {ScanState::kSucceeded, wifi_ap_list_};
}

std::tuple<bool /* scanning */, const std::vector<WiFiAp>&>
GetLatestScannedWiFiApList() {
  return {WiFi.scanComplete() == WIFI_SCAN_RUNNING, wifi_ap_list_};
}

}  // namespace wifi
