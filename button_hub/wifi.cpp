#include "wifi.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiAP.h>

#include "common.hpp"

static constexpr int kWiFiTimeout = 60 * 1000;

bool ConnectToWiFi(const char* ssid, const char* password,
                   const std::function<bool()>& giveup) {
  Serial.print("Connecting to Wi-Fi network: ");

  // Choose the best WiFi AP according to signal strength
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);

  const auto now = millis();
  WiFi.begin(ssid, password);
  do {
    if (giveup()) {
      Serial.println("Giving up connection");
      WiFi.disconnect();
      return false;
    }
    if (millis() - now > kWiFiTimeout) {
      Serial.println("Connection timeout");
      ESP.restart();
    }
    vTaskDelay(1);
  } while (WiFi.status() != WL_CONNECTED);
  Serial.printf("Connected: %s\n", GetIPAddress().c_str());
  return true;
}

bool IsWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String GetIPAddress() {
  return WiFi.localIP().toString();
}

String SetupWiFiAp(const char* ssid, const char* password) {
  WiFi.softAP(ssid, password);
  Serial.print("Hub IP: ");
  Serial.println(WiFi.softAPIP());

  return WiFi.softAPIP().toString();
}
