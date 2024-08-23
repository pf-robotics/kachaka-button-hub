#include "bluetooth_peripheral.hpp"

#include <ArduinoJson.h>
#include <ESPping.h>
#include <M5Unified.h>
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <memory.h>

#include "beep.hpp"
#include "bluetooth.hpp"
#include "ip_resolver.hpp"
#include "lgfx/v1/lgfx_fonts.hpp"
#include "screen.hpp"
#include "settings.hpp"
#include "wifi.hpp"

static constexpr char const* kServiceUuid =
    "B9E182A3-829C-4D6F-8237-05A11AF5E7B1";
static constexpr char const* kCharacteristicInitialSettingsUuid =
    "FB5018AC-AE1C-40A9-BA1D-247FFE1C6A2A";
static constexpr char const* kCharacteristicResetAllUuid =
    "C38CB086-A1AD-49E2-8ED6-DEB76F1A85FA";

static constexpr int kWiFiConnectionCheckTimeoutMs = 10 * 1000;

static bool g_is_began = false;
static bool g_is_device_connected = false;

NimBLEServer* g_ble_server = nullptr;
std::unique_ptr<NimBLEService> g_service;
std::unique_ptr<NimBLECharacteristic> g_characteristic_initial_setting;
std::unique_ptr<NimBLECharacteristic> g_characteristic_reset_all;

enum class InitialSettingResult : int {
  kSucceeded = 0,
  kInvalidInput = 1,
  kWifiConnectionFailed = 2,
  kPingToRobotFailed = 3,
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* server) {
    if (server == g_ble_server) {
      g_is_device_connected = true;
    }
  }

  void onDisconnect(NimBLEServer* server) {
    if (server == g_ble_server) {
      g_is_device_connected = false;
      server->startAdvertising();
    }
  }
};

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* characteristic) {
    if (characteristic == g_characteristic_reset_all.get()) {
      std::string value = characteristic->getValue();
      ProcessResetAllCommand(value);
    } else if (characteristic == g_characteristic_initial_setting.get()) {
      std::string value = characteristic->getValue();

      beep::PlayInitialSetupBleCommandReceived();

      InitialSettingResult result = ProcessInitialSetting(value);
      bool success = result == InitialSettingResult::kSucceeded;

      JsonDocument doc;
      doc["success"] = success;
      doc["error_code"] = static_cast<int>(result);
      std::string response;
      serializeJson(doc, response);
      characteristic->notify(response, true);

      if (result != InitialSettingResult::kSucceeded) {
        beep::PlayInitialSetupBleCommandRejected();
      } else {
        screen::DrawWhiteTextWithBlackScreen(
            {"WiFiの接続確認完了。", "再起動します ..."});
        beep::PlayInitialSetupBleCommandCompleted();
        delay(1000);
        ESP.restart();
      }
    }
  }

 private:
  void ProcessResetAllCommand(const std::string& value) {
    unsigned char binaryData = 0x01;
    if (value.size() == 1 &&
        static_cast<unsigned char>(value[0]) == binaryData) {
      Serial.println("Clearing settings...");
      g_settings.Reset();
      Serial.println("Completed");
      screen::DrawWhiteTextWithBlackScreen(
          {"設定を消去しました。", "再起動します ..."});
      beep::PlayClearAllDataCompleted();
      delay(100);
      ESP.restart();
    }
  }

  // return 0: success, 1: invalid value, 2: failed to connect to WiFi
  InitialSettingResult ProcessInitialSetting(const std::string& value) {
    if (value.length() <= 0) {
      return InitialSettingResult::kInvalidInput;
    }

    JsonDocument doc;
    deserializeJson(doc, value);

    const char* ssid = doc["ssid"];
    const char* password = doc["password"];
    const char* robot_serial = doc["robot_serial"];
    const char* robot_ip = doc["robot_ip"];

    if (ssid == nullptr || password == nullptr || robot_serial == nullptr) {
      return InitialSettingResult::kInvalidInput;
    }

    wifi::ConnectState state = wifi::ConnectToWiFi(
        ssid, password, kWiFiConnectionCheckTimeoutMs, false);
    if (state != wifi::ConnectState::kConnected) {
      // WiFi connection failed
      return InitialSettingResult::kWifiConnectionFailed;
    } else {
      // WiFi connection succeeded. Check if we can ping to the robot
      const String ip_or_host =
          ip_resolver::GetIpAddressIfPossible(robot_serial, false);
      if (!Ping.ping(ip_or_host.c_str(), 1)) {
        // Ping failed (robot_serial). Fallback to robot_ip
        if (robot_ip == nullptr || !Ping.ping(robot_ip, 1)) {
          // Ping failed (robot_ip)
          return InitialSettingResult::kPingToRobotFailed;
        } else {
          // Ping succeeded (robot_ip)
          g_settings.SetWiFiSsid(ssid);
          g_settings.SetWiFiPass(password);
          g_settings.SetRobotHost(robot_ip);
          return InitialSettingResult::kSucceeded;
        }
      } else {
        // Ping succeeded (robot_serial)
        g_settings.SetWiFiSsid(ssid);
        g_settings.SetWiFiPass(password);
        g_settings.SetRobotHost(robot_serial);
        return InitialSettingResult::kSucceeded;
      }
    }
  }
};

std::unique_ptr<ServerCallbacks> g_server_callback;
std::unique_ptr<CharacteristicCallbacks> g_characteristic_callback;

namespace bluetooth_peripheral {

bool IsDeviceConnected() {
  return g_is_device_connected;
}

void Begin() {
  if (g_is_began) {
    return;
  }
  g_is_began = true;

  bluetooth::Init();

  g_server_callback = std::unique_ptr<ServerCallbacks>(new ServerCallbacks());

  g_ble_server = NimBLEDevice::createServer();
  g_ble_server->setCallbacks(g_server_callback.get());

  g_service =
      std::unique_ptr<NimBLEService>(g_ble_server->createService(kServiceUuid));

  g_characteristic_callback =
      std::unique_ptr<CharacteristicCallbacks>(new CharacteristicCallbacks());

  g_characteristic_initial_setting =
      std::unique_ptr<NimBLECharacteristic>(g_service->createCharacteristic(
          kCharacteristicInitialSettingsUuid, NIMBLE_PROPERTY::NOTIFY |
                                                  NIMBLE_PROPERTY::WRITE |
                                                  NIMBLE_PROPERTY::WRITE_ENC));
  g_characteristic_initial_setting->setCallbacks(
      g_characteristic_callback.get());

  g_characteristic_reset_all =
      std::unique_ptr<NimBLECharacteristic>(g_service->createCharacteristic(
          kCharacteristicResetAllUuid,
          NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC));
  g_characteristic_reset_all->setCallbacks(g_characteristic_callback.get());

  g_service->start();

  NimBLEAdvertising* advertise = g_ble_server->getAdvertising();
  advertise->addServiceUUID(kServiceUuid);
  advertise->start();
}

void Stop() {
  if (!g_is_began) {
    return;
  }

  if (g_characteristic_initial_setting != nullptr && g_service != nullptr) {
    g_service->removeCharacteristic(g_characteristic_initial_setting.get());
    g_characteristic_initial_setting.reset();
  }

  if (g_characteristic_reset_all != nullptr && g_service != nullptr) {
    g_service->removeCharacteristic(g_characteristic_reset_all.get());
    g_characteristic_reset_all.reset();
  }

  if (g_service != nullptr && g_ble_server != nullptr) {
    g_ble_server->removeService(g_service.get());
    g_service.reset();
  }

  if (g_ble_server != nullptr) {
    g_ble_server->getAdvertising()->stop();
    g_ble_server = nullptr;
  }

  g_characteristic_callback.reset();
  g_server_callback.reset();

  g_is_began = false;
}

}  // namespace bluetooth_peripheral
