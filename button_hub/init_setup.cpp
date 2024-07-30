#include "init_setup.hpp"

#include <M5Unified.h>
#include <WiFi.h>

#include "beep.hpp"
#include "lgfx/v1/lgfx_fonts.hpp"
#include "qrcode.hpp"
#include "server.hpp"
#include "to_json.hpp"
#include "wifi.hpp"

InitialSetup::InitialSetup()
    : prev_state_(State::kInit), curr_state_(State::kInit) {}

void InitialSetup::RunLoop() {
  State next_state = curr_state_;
  switch (curr_state_) {
    case State::kInit: {
      next_state = State::kWaitingForConnection;
      DrawScreen(next_state);
      beep::PlayInitialSetupNext();
    } break;
    case State::kWaitingForConnection:
      if (WiFi.softAPgetStationNum() > 0) {
        next_state = State::kWaitingForSettings;
        DrawScreen(next_state);
        beep::PlayInitialSetupNext();
        wifi::StartApScan();
        server::EnqueueWsMessage(to_json::ConvertWiFiApList(true, {}));
      }
      break;
    case State::kWaitingForSettings:
      // Wait for the result of the Wi-Fi AP scan
      {
        const auto& [state, wifi_ap_list] = wifi::GetScannedWiFiApList();
        switch (state) {
          case wifi::ScanState::kScanning:
          case wifi::ScanState::kFailed:
            break;
          case wifi::ScanState::kSucceeded:
            Serial.println("WiFi AP list:");
            for (const auto& ap : wifi_ap_list) {
              Serial.printf(" - %-24s %s %3d %d\n", ap.ssid.c_str(),
                            ap.bssid.c_str(), ap.channel, ap.encryption_type);
            }
            server::EnqueueWsMessage(
                to_json::ConvertWiFiApList(false, wifi_ap_list));
            break;
        }
      }
      if (WiFi.softAPgetStationNum() == 0) {
        next_state = State::kWaitingForConnection;
        DrawScreen(next_state);
        beep::PlayInitialSetupPrev();
      }
      break;
  }

  // Transition
  if (next_state != curr_state_) {
    prev_state_ = curr_state_;
    curr_state_ = next_state;
  }

  // Reboot if any button is pressed
  M5.update();
  if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setFont(&lgfx::fonts::lgfxJapanGothicP_24);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.drawString("再起動します", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
    ESP.restart();
  }
}

/* static */ void InitialSetup::DrawScreen(const State state) {
  switch (state) {
    case State::kInit:
      M5.Lcd.fillScreen(TFT_RED);
      break;
    case State::kWaitingForConnection: {
      M5.Lcd.fillScreen(TFT_DARKGREEN);
      M5.Lcd.setFont(&lgfx::fonts::lgfxJapanGothicP_24);
      M5.Lcd.setTextDatum(TL_DATUM);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("スマートフォンのWi-Fiを");
      M5.Lcd.print(" SSID: ");
      M5.Lcd.setTextColor(TFT_CYAN);
      M5.Lcd.println(kApSsid);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.print(" パスワード: ");
      M5.Lcd.setTextColor(TFT_CYAN);
      M5.Lcd.println(kApPass);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.drawString("に接続してく", 160, 27 * 3);
      M5.Lcd.drawString("ださい。", 160, 27 * 4);
      M5.Lcd.drawString("QRコードを使", 160, 27 * 5);
      M5.Lcd.drawString("うと簡単です", 160, 27 * 6);
      const String code =
          String("WIFI:S:") + kApSsid + ";T:WPA;P:" + kApPass + ";H:false;;";
      qrcode::ShowQrCode(code.c_str(), 0.0 /* left */);
      // MAC address
      M5.Lcd.setFont(&fonts::Font2);
      M5.Lcd.setTextDatum(BC_DATUM);
      M5.Lcd.setTextColor(M5.Lcd.color565(0x80, 0xFF, 0x80));
      M5.Lcd.drawString(WiFi.macAddress(), (160 + M5.Lcd.width()) / 2,
                        M5.Lcd.height() - 2);
      break;
    }
    case State::kWaitingForSettings: {
      M5.Lcd.fillScreen(TFT_NAVY);
      M5.Lcd.setFont(&lgfx::fonts::lgfxJapanGothicP_24);
      M5.Lcd.setTextDatum(TL_DATUM);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println(
          "スマートフォンでQRコードをスキャンし、設定ページを開いてください。");
      M5.Lcd.setFont(&fonts::Font2);
      M5.Lcd.setTextDatum(TL_DATUM);
      const String url =
          String("http://") + WiFi.softAPIP().toString() + String("/#wifi");
      M5.Lcd.setTextColor(TFT_CYAN);
      M5.Lcd.print("\n ");
      M5.Lcd.println(url);
      qrcode::ShowQrCode(url.c_str(), 1.0 /* right */);
      break;
    }
  }
}
