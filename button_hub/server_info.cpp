#include "server_info.hpp"

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "beep.hpp"
#include "gpio_button.hpp"
#include "ota.hpp"
#include "server.hpp"
#include "settings.hpp"
#include "to_json.hpp"
#include "version.hpp"

void HandleGetRobotHost(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["robot_host"] = g_settings.GetRobotHost();
  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleSetRobotHost(AsyncWebServerRequest* request, const String& body) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("robot_host")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const String& robot_host = doc["robot_host"].as<String>();
  g_settings.SetRobotHost(robot_host.c_str());
  server::EnqueueWsMessage(to_json::ConvertSettings(g_settings));
  request->send(203);

  // reboot in 1 second
  delay(1000);
  ESP.restart();
}

void HandleGetWiFi(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["ssid"] = g_settings.GetWiFiSsid();
  doc["pass"] = g_settings.GetWiFiPass();
  doc["ip_address"] = g_settings.GetNetworkIpAddress();
  doc["subnet_mask"] = g_settings.GetNetworkSubnetMask();
  doc["gateway"] = g_settings.GetNetworkGateway();
  doc["dns_server_1"] = g_settings.GetNetworkDnsServer1();
  doc["dns_server_2"] = g_settings.GetNetworkDnsServer2();
  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleSetWiFi(AsyncWebServerRequest* request, const String& body) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("ssid") || !doc.containsKey("pass")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  g_settings.SetWiFiSsid(doc["ssid"].as<String>());
  g_settings.SetWiFiPass(doc["pass"].as<String>());
  g_settings.SetNetworkIpAddress(
      doc.containsKey("ip_address") ? doc["ip_address"].as<String>() : "");
  g_settings.SetNetworkSubnetMask(
      doc.containsKey("subnet_mask") ? doc["subnet_mask"].as<String>() : "");
  g_settings.SetNetworkGateway(
      doc.containsKey("gateway") ? doc["gateway"].as<String>() : "");
  g_settings.SetNetworkDnsServer1(
      doc.containsKey("dns_server_1") ? doc["dns_server_1"].as<String>() : "");
  g_settings.SetNetworkDnsServer2(
      doc.containsKey("dns_server_2") ? doc["dns_server_2"].as<String>() : "");

  server::EnqueueWsMessage(to_json::ConvertSettings(g_settings));

  request->send(203);

  beep::PlayInitialSetupComplete();

  // reboot in 1 second
  delay(1000);
  ESP.restart();
}

void HandleGetBeepVolume(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["beep_volume"] = g_settings.GetBeepVolume();
  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleSetBeepVolume(AsyncWebServerRequest* request, const String& body) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("beep_volume")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const int beep_volume = doc["beep_volume"].as<int>();
  if (0 <= beep_volume && beep_volume <= 11) {
    g_settings.SetBeepVolume(beep_volume);
    beep::SetVolume(beep_volume);
    server::EnqueueWsMessage(to_json::ConvertSettings(g_settings));
    request->send(203);
  } else {
    request->send(400, "text/plain", "Invalid range of beep_volume");
  }
}

void HandleGetScreenBrightness(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["screen_brightness"] = g_settings.GetScreenBrightness();
  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleSetScreenBrightness(AsyncWebServerRequest* request,
                               const String& body) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("screen_brightness")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const int v = doc["screen_brightness"].as<int>();
  if (0 <= v && v <= 255) {
    g_settings.SetScreenBrightness(v);
    M5.Lcd.setBrightness(v);
    server::EnqueueWsMessage(to_json::ConvertSettings(g_settings));
    request->send(203);
  } else {
    request->send(400, "text/plain", "Invalid range of screen_brightness");
  }
}

void HandleGetDesiredHubVersion(AsyncWebServerRequest* request) {
  JsonDocument doc;

  const String version = ota::GetDesiredHubVersion(g_settings.GetOtaEndpoint());
  if (version.isEmpty()) {
    doc["success"] = false;
  } else {
    const bool required = ota::CheckOtaIsRequired(kVersion, version);
    doc["success"] = true;
    doc["desired_hub_version"] = std::move(version);
    doc["ota_is_required"] = required;
  }

  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleGetOtaImageUrlByVersion(AsyncWebServerRequest* request,
                                   const String& body) {
  String version;
  {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
      Serial.println("ERROR: Failed to parse JSON");
      request->send(400, "text/plain", "Bad Request");
      return;
    }
    if (!doc.containsKey("version")) {
      Serial.println("ERROR: Invalid JSON");
      request->send(400, "text/plain", "Bad Request");
      return;
    }
    version = doc["version"].as<String>();
  }
  {
    String url =
        ota::GetOtaImageUrlByVersion(g_settings.GetOtaEndpoint(), version);
    if (url.isEmpty()) {
      request->send(500, "text/plain", "Failed to get OTA image URL");
      return;
    }
    JsonDocument doc;
    doc["url"] = std::move(url);
    String out;
    serializeJson(doc, out);
    request->send(200, "text/json; charset=utf-8", out);
  }
}

void HandleGetAutoOtaIsEnabled(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["auto_ota_is_enabled"] = g_settings.GetAutoOtaIsEnabled();
  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleSetAutoOtaIsEnabled(AsyncWebServerRequest* request,
                               const String& body) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("auto_ota_is_enabled")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const bool v = doc["auto_ota_is_enabled"].as<bool>();
  g_settings.SetAutoOtaIsEnabled(v);
  server::EnqueueWsMessage(to_json::ConvertSettings(g_settings));
  request->send(203);
}

void HandleGetOneShotAutoOtaIsEnabled(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["one_shot_auto_ota_is_enabled"] = g_settings.GetOneShotAutoOtaIsEnabled();
  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleSetOneShotAutoOtaIsEnabled(AsyncWebServerRequest* request,
                                      const String& body) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("one_shot_auto_ota_is_enabled")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const bool v = doc["one_shot_auto_ota_is_enabled"].as<bool>();
  g_settings.SetOneShotAutoOtaIsEnabled(v);
  request->send(203);
}

void HandleGetAutoRefetchOnUiLoad(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["auto_refetch_on_ui_load"] = g_settings.GetAutoRefetchOnUiLoad();
  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleSetAutoRefetchOnUiLoad(AsyncWebServerRequest* request,
                                  const String& body) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("auto_refetch_on_ui_load")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const bool v = doc["auto_refetch_on_ui_load"].as<bool>();
  g_settings.SetAutoRefetchOnUiLoad(v);
  server::EnqueueWsMessage(to_json::ConvertSettings(g_settings));
  request->send(203);
}

void HandleGetGpioButtonIsEnabled(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["gpio_button_is_enabled"] = g_settings.GetGpioButtonIsEnabled();
  String out;
  serializeJson(doc, out);
  request->send(200, "text/json; charset=utf-8", out);
}

void HandleSetGpioButtonIsEnabled(AsyncWebServerRequest* request,
                                  const String& body,
                                  CommandTable& command_table) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("gpio_button_is_enabled")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const bool v = doc["gpio_button_is_enabled"].as<bool>();
  g_settings.SetGpioButtonIsEnabled(v);
  server::EnqueueWsMessage(to_json::ConvertSettings(g_settings));
  request->send(203);
}

void HandleOtaByImageUrl(AsyncWebServerRequest* request, const String& body) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  if (!doc.containsKey("url")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const String& url = doc["url"].as<String>();
  if (url.isEmpty() || !url.startsWith("http")) {
    request->send(400, "text/plain", "Invalid url");
    return;
  }
  request->send(203, "text/plain", "OTA process has been started");
  ota::RebootForOtaAfterBoot(url);
}

void HandleClearAllData(AsyncWebServerRequest* request,
                        CommandTable& command_table) {
  Serial.println("Clearing settings...");
  g_settings.Reset();
  Serial.println("Clearing commands...");
  command_table.Reset();
  Serial.println("Completed");
  request->send(203);

  beep::PlayClearAllDataCompleted();

  // reboot in 1 second
  delay(1000);
  ESP.restart();
}
