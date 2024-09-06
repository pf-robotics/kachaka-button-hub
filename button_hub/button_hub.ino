#include <M5Unified.h>
#include <SPIFFS.h>
#include <TickTwo.h>
#include <memory>

#include "api.hpp"
#include "api_mutex.hpp"
#include "beep.hpp"
#include "bluetooth.hpp"
#include "bluetooth_beacon.hpp"
#include "bluetooth_peripheral.hpp"
#include "command_table.hpp"
#include "fetch_state.hpp"
#include "gpio_button.hpp"
#include "init_setup.hpp"
#include "ip_resolver.hpp"
#include "logging.hpp"
#include "mutex.hpp"
#include "ota.hpp"
#include "ping_to_robot.hpp"
#include "screen.hpp"
#include "send_command.hpp"
#include "server.hpp"
#include "settings.hpp"
#include "to_json.hpp"
#include "types.hpp"
#include "version.hpp"
#include "wifi.hpp"

constexpr int kButtonIgnoreDurationSec = 11;
constexpr int kMaxObservedButtonCount = 10;
constexpr int kMaxAutoOtaTrialCount = 3;

constexpr int kBluetoothBeaconSetupIntervalMsec = 1 * 1000;
constexpr int kWiFiSignalUpdateIntervalMSec = 1 * 1000;
constexpr int kRebootCheckIntervalMSec = 60 * 1000;
constexpr int kDrawClockIntervalMSec = 2 * 1000;
constexpr int kCountDownRebootIntervalMSec = 1 * 1000;

static Preferences g_prefs;
static RobotInfoHolder g_robot;
static CommandTable g_command_table(kMaxObservedButtonCount);

enum class Page {
  kMain = 0,
  kSetting = 1,
  kOta = 2,
};
static Page g_page = Page::kMain;

static int g_reboot_count_down = -1;

static std::map<KButton, time_t> g_last_beacon_time;
kb::Mutex g_button_queue_mutex;
static std::deque<std::pair<KButton, double>> g_button_queue;

bool IsBraveridgeBeacon(const uint8_t uuid[16]) {
  for (int i = 0; i < sizeof(uuid); i++) {
    if (uuid[i] != i) {
      return false;
    }
  }
  return true;
}

static void BeaconCallback(const char* name, const uint8_t address[6],
                           const uint8_t uuid[16], const uint16_t major,
                           const uint16_t minor, const int8_t tx_power,
                           const int rssi) {
  if (!IsBraveridgeBeacon(uuid)) {
    return;
  }
  char beacon_str[128];
  snprintf(
      beacon_str, sizeof(beacon_str),
      "Beacon ==> %s "
      "%02x:%02x:%02x:%02x:%02x:%02x, "
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x, "
      "%04x, %04x, %d, %d",
      name, address[0], address[1], address[2], address[3], address[4],
      address[5], uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6],
      uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13],
      uuid[14], uuid[15], major, minor, tx_power, rssi);
  logging::Log("Beacon: %s", beacon_str);

  const double estimated_distance = std::pow(10.0, (tx_power - rssi) / 20.0);

  const KButton button(AppleIBeacon(address, uuid, major, minor));

  if (const kb::LockGuard lock(g_button_queue_mutex); lock) {
    g_button_queue.emplace_back(button, estimated_distance);
  } else {
    logging::Log("Beacon: Discarding button event due to lock failure");
  }
}

static void HandleButtonPressed(const KButton& button,
                                const double estimated_distance) {
  if (button.type == ButtonType::kAppleIBeacon) {
    const time_t now = time(nullptr);
    const auto it = g_last_beacon_time.find(button);
    if (it != g_last_beacon_time.end()) {
      const int diff = now - it->second;
      if (diff < kButtonIgnoreDurationSec) {
        return;
      }
    }
    g_last_beacon_time[button] = now;
  }

  g_command_table.NotifyObservedButton(button, estimated_distance);
  server::EnqueueWsMessage(to_json::ConvertObservedButtons(
      g_command_table.GetObservedButtons(), g_command_table.GetButtonNames()));

  Command command;
  if (g_command_table.GetCommandByButton(button, &command)) {
    if (const kb::LockGuard lock(api_mutex); lock) {
      logging::Log("Button pressed: %s",
                   to_json::ConvertCommand(command).c_str());
      beep::PlayCommandSent();
      screen::DrawCommandSent(true);
      bool ok = send_command::SendCommand(g_robot, command);
      if (!ok) {
        beep::PlayCommandFailed();
      }
      screen::DrawCommandSent(false);
    }
  }
}

void RegisterOrUnregisterGpioButtonAccordingToSettings(
    bool gpio_button_is_enabled, CommandTable& command_table) {
  static bool prev = false;
  if (gpio_button_is_enabled != prev) {
    if (gpio_button_is_enabled) {
      gpio_button::Register(command_table);
    } else {
      gpio_button::Unregister(command_table);
    }
    prev = gpio_button_is_enabled;
  }
}

static void SendWifiRssi() {
  const int rssi = WiFi.RSSI();
  screen::DrawWiFiSignalStrength(WiFi.status() == WL_CONNECTED, rssi);
  server::EnqueueWsMessage(
      "{\"type\":\"wifi_rssi\",\"wifi_rssi\":" + String(rssi) + "}");
}

static void CheckReboot() {
  // a.m. 5:00
  static int prev_hour = -1;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) {
    return;
  }
  if (prev_hour == 4 && timeinfo.tm_hour == 5) {
    screen::DrawWhiteTextWithBlackScreen({"朝5時の定期再起動", "を実行します"});
    logging::Log("Rebooting due to a.m. 5 ...");
    logging::Update();
    delay(1000);
    ESP.restart();
  }
  prev_hour = timeinfo.tm_hour;

  // WiFi connection
  static int64_t last_connected_time = millis();
  if (WiFi.status() == WL_CONNECTED) {
    last_connected_time = millis();
  } else {
    if (millis() - last_connected_time > 3 * 60 * 1000) {
      screen::DrawWhiteTextWithBlackScreen(
          {"Wi-Fi接続が切れたまま", "復帰できないので", "再起動します"});
      logging::Log("Rebooting due to disconnection ...");
      logging::Update();
      delay(1000);
      ESP.restart();
    }
  }

  // Fetch
  if (!fetch_state::IsCompleted()) {
    if (fetch_state::GetDurationFromLastStart() > 3 * 60 * 1000) {
      screen::DrawWhiteTextWithBlackScreen(
          {"ロボットからの情報取得", "が長引いているので", "再起動します"});
      logging::Log("Rebooting due to fetch timeout ...");
      logging::Update();
      delay(1000);
      ESP.restart();
    }
  }

  // Ping
  if (ping_to_robot::GetDurationFromLastSuccess() > 3 * 60 * 1000) {
    screen::DrawWhiteTextWithBlackScreen(
        {"ロボットへのpingが", "失敗し続けているので", "再起動します"});
    logging::Log("Rebooting due to ping failure ...");
    logging::Update();
    delay(1000);
    ESP.restart();
  }

  // Stats
  Serial.printf("Free heap: now=%6.1f kb, min=%6.1f kb\n",
                esp_get_free_heap_size() / 1000.0,
                esp_get_minimum_free_heap_size() / 1000.0);
}

static void CountDownReboot() {
  if (g_reboot_count_down < 0) {
    return;
  }
  if (--g_reboot_count_down <= 0) {
    logging::Log("Rebooting ...");
    logging::Update();
    ESP.restart();
  }
  logging::Log("Count down ... %d", g_reboot_count_down);
}

static void InitSpiffs() {
  if (!SPIFFS.begin(/* format_spiffs_if_failed= */ false)) {
    M5.Lcd.println("");
    M5.Lcd.setTextFont(4);
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.println("Formatting SPIFFS ...");
    M5.Lcd.println("Do not turn off the power");
    Serial.println("Formatting SPIFFS ...");
    SPIFFS.format();
    M5.Lcd.println("Done");
    Serial.println("Done");
    delay(1000);
    ESP.restart();
  }
}

static bluetooth_beacon::SetupTicker g_bluetooth_beacon_setup(
    kBluetoothBeaconSetupIntervalMsec,
    []() {
      return g_robot.has_robot_version && g_robot.has_shelves &&
             g_robot.has_locations && g_robot.has_shortcuts;
    },
    &BeaconCallback);
static TickTwo g_wifi_rssi_timer(SendWifiRssi, kWiFiSignalUpdateIntervalMSec);
static TickTwo g_reboot_timer(CheckReboot, kRebootCheckIntervalMSec);
static TickTwo g_clock_timer(screen::DrawClock, kDrawClockIntervalMSec);
static TickTwo g_count_down_reboot_timer(CountDownReboot,
                                         kCountDownRebootIntervalMSec);

enum class Mode {
  kUnknown,
  kInitialSetup,
  kButtonServer,
  kOtaAfterBoot,
};
static Mode g_mode = Mode::kUnknown;
static InitialSetup g_initial_setup;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Power.begin();
  M5.Lcd.setTextFont(2);
  M5.Lcd.println(kVersion);

  Serial.begin(115200);
  Serial.println("Ok");

  InitSpiffs();

  if (!g_prefs.begin("kachaka", false)) {
    Serial.println("Failed to open preferences");
  }
  g_settings.Begin(&g_prefs);

  beep::Begin(g_settings.GetBeepVolume());
  screen::Begin(g_settings.GetScreenBrightness());

  logging::Begin(g_settings.GetNextLoggingId());
  logging::Log("Start");

  bluetooth::Init();

  const wifi::ConnectState wifi_connect_state =
      wifi::ConnectToWiFi(g_settings.GetWiFiSsid().c_str(),
                          g_settings.GetWiFiPass().c_str(), 60 * 1000, true);
  if (wifi_connect_state == wifi::ConnectState::kTimeout) {
    ESP.restart();
  } else if (wifi_connect_state == wifi::ConnectState::kConnected) {
    screen::DrawCheckingUpdate();
    if (!g_settings.GetNtpServer().isEmpty()) {
      configTime(9 * 3600, 0, g_settings.GetNtpServer().c_str());
    }
    // Pre-fetch desired version
    ota::GetDesiredHubVersion(g_settings.GetOtaEndpoint());
    // Manual OTA?
    String ota_url = g_settings.GetOtaUrlAfterBoot();
    if (!ota_url.isEmpty()) {
      g_settings.SetOtaUrlAfterBoot("");
    }
    // Auto OTA?
    if ((g_settings.GetAutoOtaIsEnabled() ||
         g_settings.GetOneShotAutoOtaIsEnabled()) &&
        ota_url.isEmpty()) {
      ota_url =
          ota::GetOtaImageUrlFromServerIfAvailable(g_settings.GetOtaEndpoint());
      if (g_settings.GetOtaFailCount() >= kMaxAutoOtaTrialCount) {
        Serial.println("Auto OTA is turned off due to failure count");
        g_settings.SetAutoOtaIsEnabled(false);
        g_settings.ClearOtaFailCount();
        ota_url.clear();
      }
      g_settings.SetOneShotAutoOtaIsEnabled(false);
    }
    // OTA or App
    if (!ota_url.isEmpty()) {
      SetupAsOtaAfterBoot(ota_url);
      g_mode = Mode::kOtaAfterBoot;
    } else {
      SetupAsWiFiClient();
      g_mode = Mode::kButtonServer;
    }
  } else {
    WiFi.softAP(kApSsid, kApPass);
    Serial.print("Hub IP: ");
    Serial.println(WiFi.softAPIP());

    bluetooth_peripheral::Begin();

    server::SetupHttpServerForWiFiSetting(g_robot, g_command_table);

    g_mode = Mode::kInitialSetup;
  }
}

static void DrawScreen() {
  static kb::Mutex mutex;
  const kb::LockGuard lock(mutex);
  switch (g_page) {
    case Page::kMain:
      screen::DrawMainPage(g_settings.GetWiFiSsid().c_str(),
                           wifi::GetIpAddress().c_str(),
                           g_settings.GetRobotHost().c_str());
      break;
    case Page::kSetting:
      screen::DrawSettingPage(wifi::GetIpAddress().c_str());
      break;
    case Page::kOta:
      screen::DrawOtaPage();
      break;
  }
}

static void SetupOta() {
  ota::Begin(
      []() {  // on_start
        server::Stop();
        bluetooth_beacon::Stop();
        bluetooth_peripheral::Stop();
        ping_to_robot::Stop();
        g_page = Page::kOta;
        DrawScreen();
        // Increment fail count here to avoid the case where the device
        // reboots without incrementing the fail count.
        g_settings.IncrementOtaFailCount();
      },
      [](double percent) {  // on_progress
        screen::DrawOtaProgress(percent);
      },
      [](bool success) {  // on_end
        if (success) {
          beep::PlayOtaCompleted();
          g_settings.ClearOtaFailCount();
          delay(1000);
          ESP.restart();
        } else {
          beep::PlayOtaFailed();
          g_reboot_count_down = 60;
          g_count_down_reboot_timer.start();
        }
      },
      [](ota::Error error) {  // on_error
        screen::DrawOtaError(error);
        if (error == ota::Error::kWatchdogNow) {
          Serial.printf("OTA fail count: %d\n", g_settings.GetOtaFailCount());
          beep::PlayOtaFailed();
          delay(1000);
          ESP.restart();
        }
      });
}

static void SetupAsOtaAfterBoot(const String& ota_url) {
  g_page = Page::kOta;
  DrawScreen();
  SetupOta();
  g_wifi_rssi_timer.start();
  g_clock_timer.start();
  ota::StartOtaByUrl(ota_url);
}

static void SetupAsWiFiClient() {
  if (g_settings.GetRobotHost().isEmpty()) {
    g_page = Page::kSetting;
  }
  DrawScreen();

  ip_resolver::Begin();

  g_wifi_rssi_timer.start();
  ping_to_robot::Begin(g_settings.GetRobotHost().c_str());

  g_command_table.Load();

  server::SetupHttpServer(g_robot, g_command_table);

  api::SetRobotHost(g_settings.GetRobotHost(), 26400);
  fetch_state::FetchRobotInfo(&g_robot);

  g_command_table.SetButtonName(KButton(M5Button(2)), "HubボタンA");
  g_command_table.SetButtonName(KButton(M5Button(3)), "HubボタンB");
  g_command_table.Save();
  server::EnqueueWsMessage(to_json::ConvertObservedButtons(
      g_command_table.GetObservedButtons(), g_command_table.GetButtonNames()));

  g_reboot_timer.start();
  g_clock_timer.start();

  g_bluetooth_beacon_setup.Start();
}

void loop() {
  if (g_mode == Mode::kInitialSetup) {
    g_initial_setup.RunLoop();
  } else if (g_mode == Mode::kOtaAfterBoot) {
    M5.update();
    if (M5.BtnB.wasPressed()) {
      beep::PlayOtaFailed();
      delay(250);
      ESP.restart();
    }
    g_wifi_rssi_timer.update();
    g_clock_timer.update();
    g_count_down_reboot_timer.update();
  } else if (g_mode == Mode::kButtonServer) {
    M5.update();
    bool need_redraw = false;
    if (M5.BtnA.wasPressed() &&
        (g_page == Page::kMain || g_page == Page::kSetting)) {
      beep::PlayNextPage();
      g_page = g_page == Page::kMain ? Page::kSetting : Page::kMain;
      DrawScreen();
      need_redraw = true;
    }
    switch (g_page) {
      case Page::kMain:
        if (M5.BtnB.wasPressed()) {
          HandleButtonPressed(KButton(M5Button(2)), -1);
        }
        if (M5.BtnC.wasPressed()) {
          HandleButtonPressed(KButton(M5Button(3)), -1);
        }
        gpio_button::HandleEvents([](const KButton& pressed_button) {
          HandleButtonPressed(pressed_button, -1);
        });
        screen::DrawStatusInMainPage(!fetch_state::IsCompleted(),
                                     g_robot.has_robot_version,
                                     g_robot.has_shelves, g_robot.has_locations,
                                     g_robot.has_shortcuts, need_redraw);
        ping_to_robot::Update();
        break;
      case Page::kOta:
        if (M5.BtnB.wasPressed()) {
          beep::PlayOtaFailed();
          delay(250);
          ESP.restart();
        }
    }
    if (need_redraw) {
      screen::DrawWiFiSignalStrength(WiFi.status() == WL_CONNECTED,
                                     WiFi.RSSI());
      screen::DrawClock();
    }
    if (const kb::LockGuard lock(g_button_queue_mutex); lock) {
      if (!g_button_queue.empty()) {
        const auto& [button, estimated_distance] = g_button_queue.front();
        HandleButtonPressed(button, estimated_distance);
        g_button_queue.pop_front();
      }
    } else {
      Serial.println("Failed to lock button_queue mutex");
    }
    RegisterOrUnregisterGpioButtonAccordingToSettings(
        g_settings.GetGpioButtonIsEnabled(), g_command_table);
    g_bluetooth_beacon_setup.Update();
    g_wifi_rssi_timer.update();
    g_reboot_timer.update();
    g_clock_timer.update();
  }
  server::FlushWsMessageQueue();
  logging::Update();
  delay(5);
}
