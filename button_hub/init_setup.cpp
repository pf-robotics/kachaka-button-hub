#include "init_setup.hpp"

#include <WiFi.h>

#include "beep.hpp"
#include "common.hpp"
#include "lgfx/v1/lgfx_fonts.hpp"
#include "qrcode.hpp"

InitialSetup::InitialSetup()
    : prev_state_(State::kInit), next_state_(State::kWaitForWiFiClient) {}

void InitialSetup::RunLoop() {
  switch (next_state_) {
    case State::kInit:
      // Do something
      break;
    case State::kWaitForWiFiClient:
      if (prev_state_ == State::kInit) {
        WiFi.softAP(kApSsid, kApPass);
        Serial.print("Hub IP: ");
        Serial.println(WiFi.softAPIP());
        DrawScreen(next_state_);
        beep::PlayInitialSetupNext();
      }
      if (WiFi.softAPgetStationNum() > 0) {
        next_state_ = State::kWaitForSettings;
        DrawScreen(next_state_);
        beep::PlayInitialSetupNext();
      }
      break;
    case State::kWaitForSettings:
      if (WiFi.softAPgetStationNum() == 0) {
        next_state_ = State::kWaitForWiFiClient;
        DrawScreen(next_state_);
        beep::PlayInitialSetupPrev();
      }
      break;
  }
  prev_state_ = next_state_;
}

/* static */ void InitialSetup::DrawScreen(const State state) {
  switch (state) {
    case State::kInit:
      M5.Lcd.fillScreen(TFT_RED);
      break;
    case State::kWaitForWiFiClient: {
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
    case State::kWaitForSettings: {
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
