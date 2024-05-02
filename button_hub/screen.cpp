#include "screen.hpp"

#include <cmath>

#include "common.hpp"
#include "mutex.hpp"
#include "settings.hpp"
#include "version.hpp"

namespace {

const uint16_t kKachakaBlack = M5.Lcd.color565(0x1C, 0x27, 0x42);
const uint16_t kKachakaGray1 = M5.Lcd.color565(0xEA, 0xEE, 0xF6);
const uint16_t kKachakaGray2 = M5.Lcd.color565(0xCE, 0xD6, 0xEA);
const uint16_t kKachakaGray3 = M5.Lcd.color565(0xA4, 0xB0, 0xCA);
const uint16_t kKachakaGray4 = M5.Lcd.color565(0x8D, 0x98, 0xB2);
const uint16_t kKachakaGray5 = M5.Lcd.color565(0x57, 0x65, 0x85);
const uint16_t kStatusSuccess = M5.Lcd.color565(0x47, 0xCC, 0x9C);
const uint16_t kStatusDanger = M5.Lcd.color565(0xFF, 0x5D, 0x5D);

constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 240;
constexpr int kMargin = 8;
constexpr int kHeaderHeight = 22;  // y of the title border
constexpr int kInfoHeight = 138;
constexpr int kFooterHeight = kHeaderHeight + 176;
constexpr int kOtaProgressY = 115;
constexpr int kOtaProgressHeight = 16;

constexpr int kHeaderMargin = 4;
constexpr int kWiFiSignalStrengthX = 294;

const auto kFontS = &fonts::Font2;  // 16
const auto kFontL = &fonts::Font4;  // 26
const auto kFontJaS = &lgfx::fonts::efontJA_14;
const auto kFontJaSB = &lgfx::fonts::efontJA_14_b;
const auto kFontJaMB = &lgfx::fonts::efontJA_16_b;

kb::Mutex g_mutex;

}  // namespace

static void DrawWiFi(const int x, const int y) {
  const int w = 24;
  const int h = 18;
  for (int i = 0; i < 4; i++) {
    const int r = i * 5 + 2;
    M5.Lcd.fillArc(x + w / 2, y + h, r + 1, r - 1, 225.0, 315.0, kKachakaGray5);
  }
}

static void DrawM5Stack(const int x, const int y) {
  const int w = 23;
  const int h = 22;
  M5.Lcd.fillRect(x + 0, y + 0, w, h, kKachakaBlack);
  M5.Lcd.fillRect(x + 3, y + 4, 17, 12, kKachakaGray2);
  for (int i = 0; i < 3; i++) {
    M5.Lcd.fillRect(x + 5 + i * 5, y + 18, 3, 2, kKachakaGray1);
  }
}

static void DrawKachaka(int x, int y) {
  const int w = 28;
  const int h = 17;
  M5.Lcd.fillRoundRect(x, y, w, h, 3, kKachakaBlack);
  M5.Lcd.drawCircle(x + 21, y + h / 2, 3, kKachakaGray1);
  M5.Lcd.drawCircle(x + 10, y + h / 2, 2, kKachakaGray5);
}

static void DrawStringWithinWidth(const char* text, const int x, const int y,
                                  const int max_width) {
  const int w = M5.Lcd.textWidth(text);
  if (w <= max_width) {
    M5.Lcd.drawString(text, x, y);
  } else {
    M5.Lcd.setTextSize(static_cast<float>(max_width) / w, 1);
    M5.Lcd.drawString(text, x, y);
    M5.Lcd.setTextSize(1);
  }
}

static void DrawBox1(const char* ssid, const char* hub_host,
                     const char* robot_host) {
  M5.Lcd.setFont(kFontL);
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setTextColor(kKachakaBlack);

  const int x1 = 66;
  const int y1 = 22 + 8 + 6 + 3;

  DrawWiFi(24, y1);
  DrawStringWithinWidth(ssid, x1, y1, kScreenWidth - x1 - kMargin * 2);

  DrawM5Stack(24, y1 + 29);
  DrawStringWithinWidth(hub_host, x1, y1 + 29, kScreenWidth - x1 - kMargin * 2);

  DrawKachaka(22, y1 + 3 + 29 * 2);
  DrawStringWithinWidth(robot_host, x1, y1 + 29 * 2,
                        kScreenWidth - x1 - kMargin * 2);
}

static void DrawBox2() {
  M5.Lcd.setFont(kFontJaS);
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setTextColor(kKachakaBlack);
  const int lh = M5.Lcd.fontHeight() + 2;
  const int y = kInfoHeight;
  String version = "Hubのバージョン: ";
  version += kVersion;
  const String ota_label = g_settings.GetOtaLabel();
  if (!ota_label.isEmpty()) {
    version += " (";
    version += ota_label;
    version += ")";
  }
  M5.Lcd.drawString(version.c_str(), 24, y);
  M5.Lcd.drawString("ロボットからの情報取得:", 24, y + lh * 1);
  M5.Lcd.drawString("ロボットへPing:", 24, y + lh * 2);
}

static void DrawBase(const uint16_t bg_color, const char* title,
                     const char* button_a_label, const char* button_b_label,
                     const char* button_c_label) {
  M5.Lcd.fillScreen(bg_color);

  // header
  M5.Lcd.fillRect(0, 0, 320, kHeaderHeight - 1, TFT_WHITE);
  M5.Lcd.drawLine(0, kHeaderHeight, 320, kHeaderHeight, kKachakaGray3);
  M5.Lcd.setTextColor(kKachakaGray5);
  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setFont(kFontJaSB);
  M5.Lcd.drawString(title, kScreenWidth / 2, kHeaderHeight / 2);

  // buttons
  const char* labels[3] = {button_a_label, button_b_label, button_c_label};
  const int w = 68;
  const int h = 32;
  const int m = 26;
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setFont(kFontJaSB);
  for (int i = 0; i < 3; i++) {
    M5.Lcd.fillRoundRect(32 + (w + m) * i, kFooterHeight + 6, w, h, 3,
                         labels[i] != nullptr ? kKachakaGray5 : kKachakaGray3);
    if (labels[i] != nullptr) {
      M5.Lcd.drawString(labels[i], 32 + (w + m) * i + w / 2,
                        kFooterHeight + 6 + h / 2);
    }
  }
}

namespace screen {

void Begin(const int brightness) {
  M5.Lcd.setBrightness(brightness);
}

void DrawMainPage(const char* ssid, const char* hub_host,
                  const char* robot_host) {
  const kb::LockGuard lock(g_mutex);
  const auto bg = M5.Lcd.color565(0xD7, 0xD8, 0xDA);
  DrawBase(bg, "ステータス", "設定", "ボタンA", "ボタンB");

  // boxes
  const int y2 = kHeaderHeight + kMargin;  // y of the box1
  const int y3 = kHeaderHeight + 110;      // y of the box2
  M5.Lcd.fillRoundRect(kMargin, y2, kScreenWidth - kMargin * 2, 97, 6,
                       TFT_WHITE);
  M5.Lcd.fillRoundRect(kMargin, y3, kScreenWidth - kMargin * 2, 60, 6,
                       TFT_WHITE);
  DrawBox1(ssid, hub_host, robot_host);
  DrawBox2();
}

void DrawStatusInMainPage(const bool fetching, const bool has_robot_version,
                          const bool has_shelves, const bool has_locations,
                          const bool need_redraw) {
  const kb::LockGuard lock(g_mutex);
  static bool first = true;
  static bool prev_fetching = false;
  static bool prev_has_robot_version = false;
  static bool prev_has_shelves = false;
  static bool prev_has_locations = false;
  if (first || need_redraw || fetching != prev_fetching ||
      has_robot_version != prev_has_robot_version ||
      has_shelves != prev_has_shelves || has_locations != prev_has_locations) {
    M5.Lcd.setFont(kFontJaSB);
    M5.Lcd.setTextDatum(TL_DATUM);
    const int lh = M5.Lcd.fontHeight() + 2;
    const int w = M5.Lcd.textWidth("ロボットからの情報取得: ");
    M5.Lcd.setCursor(24 + w, kInfoHeight + lh * 1);
    const int ok_count = static_cast<int>(has_robot_version) +
                         static_cast<int>(has_shelves) +
                         static_cast<int>(has_locations);
    if (ok_count == 3) {
      M5.Lcd.setTextColor(kStatusSuccess, TFT_WHITE);
      if (fetching) {
        M5.Lcd.print("再取得中... ");
      } else {
        M5.Lcd.print("完了        ");
      }
    } else {
      M5.Lcd.setTextColor(kStatusDanger, TFT_WHITE);
      M5.Lcd.printf("取得中 (%d/3)", ok_count);
    }
    prev_fetching = fetching;
    prev_has_robot_version = has_robot_version;
    prev_has_shelves = has_shelves;
    prev_has_locations = has_locations;
  }
  first = false;
}

void DrawPingResult(const bool ok, const float time_ms, const int ng_count) {
  const kb::LockGuard lock(g_mutex);
  M5.Lcd.setFont(kFontJaSB);
  const int h = M5.Lcd.height();
  const int lh = M5.Lcd.fontHeight() + 2;
  const int y = kInfoHeight + lh * 2;
  const int x = 138;
  M5.Lcd.setCursor(x, y);
  if (ok) {
    M5.Lcd.setTextColor(kStatusSuccess, TFT_WHITE);
    if (0 <= time_ms && time_ms < 60 * 1000) {
      M5.Lcd.printf("%.0f ms   ", time_ms);
    } else {
      Serial.printf("Invalid ping time: %.1f ms\n", time_ms);
      M5.Lcd.printf("??? ms   ");
    }
  } else {
    M5.Lcd.setTextColor(kStatusDanger, TFT_WHITE);
    M5.Lcd.printf("NG (%d)   ", ng_count);
  }
}

void DrawCommandSent(bool sent) {
  const kb::LockGuard lock(g_mutex);
  constexpr int kRadius = 6;
  M5.Lcd.fillCircle(kWiFiSignalStrengthX - kMargin - kRadius,
                    kHeaderMargin + kRadius, kRadius,
                    sent ? kStatusDanger : TFT_WHITE);
}

void DrawSettingPage(const char* hub_host) {
  const kb::LockGuard lock(g_mutex);
  DrawBase(TFT_WHITE, "設定", "戻る", "", "");
  M5.Lcd.drawLine(0, kFooterHeight, 320, kFooterHeight, kKachakaGray3);

  // QR code
  String url = String("http://") + hub_host;
  constexpr int kQrSize = 140;
  constexpr int kQrVersion = 2;
  constexpr int kOuterMargin = 8;
  const int x = kScreenWidth - (kQrSize + kMargin);
  const int y = kHeaderHeight + kMargin * 2;
  M5.Lcd.qrcode(url.c_str(), x + kOuterMargin, y + kOuterMargin,
                kQrSize - kOuterMargin * 2, kQrVersion);
  for (int i = 0; i < 3; i++) {
    M5.Lcd.drawRoundRect(x - 1 + i, y - 1 + i, kQrSize + (2 - i * 2),
                         kQrSize + (2 - i * 2), 8, TFT_BLUE);
  }

  // message
  const int y2 = y + 4;
  M5.Lcd.setTextColor(kKachakaBlack);
  M5.Lcd.setTextDatum(TC_DATUM);

  M5.Lcd.setFont(kFontJaMB);
  M5.Lcd.drawString("カチャカボタン", x / 2, y2);
  M5.Lcd.drawString("の設定", x / 2, y2 + 24);

  M5.Lcd.setFont(kFontJaS);
  M5.Lcd.drawString("スマートフォンのカメ", x / 2, y2 + 64);
  M5.Lcd.drawString("ラでQRコードを読み取", x / 2, y2 + 64 + 20);
  M5.Lcd.drawString("ってください。", x / 2, y2 + 64 + 40);
}

void DrawOtaPage() {
  {
    const kb::LockGuard lock(g_mutex);
    DrawBase(TFT_WHITE, "システムアップデート", "", "リセット", "");
    M5.Lcd.drawLine(0, kFooterHeight, 320, kFooterHeight, kKachakaGray3);

    M5.Lcd.setTextColor(kKachakaGray4, TFT_WHITE);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setFont(kFontJaS);
    M5.Lcd.drawString("電源を切っても安全です。中断したい場合",
                      kScreenWidth / 2, kHeaderHeight + 70);
    M5.Lcd.drawString("はリセットボタンを押してください。", kScreenWidth / 2,
                      kHeaderHeight + 90);
  }
  DrawOtaProgress(0);
}

void DrawOtaProgress(double percent) {
  const kb::LockGuard lock(g_mutex);
  M5.Lcd.setTextColor(kKachakaGray5, TFT_WHITE);
  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setFont(kFontJaMB);
  char buf[64];
  snprintf(buf, sizeof(buf), "  ダウンロード中... %.1f%%  ", percent);
  M5.Lcd.drawString(buf, kScreenWidth / 2, kHeaderHeight + 40);

  M5.Lcd.progressBar(kMargin, kHeaderHeight + kOtaProgressY,
                     kScreenWidth - kMargin * 2, kOtaProgressHeight, percent);
}

void DrawOtaError(ota::Error error_code) {
  const kb::LockGuard lock(g_mutex);
  const char* msg1 = "エラーが発生しました";
  const char* msg2 = "60秒後に再起動します";
  switch (error_code) {
    case ota::Error::kNoError:
      msg1 = "";
      msg2 = "";
      break;
    case ota::Error::kHttpGetFailed:
      msg1 = "ダウンロードに失敗しました";
      break;
    case ota::Error::kNotEnoughMemory:
      msg1 = "空きメモリが不足しています";
      break;
    case ota::Error::kNotEnoughSpace:
      msg1 = "空き容量が足りません";
      break;
    case ota::Error::kWrittenOnlyPartialFile:
      msg1 = "ファイルの書き込みに失敗しました";
      break;
    case ota::Error::kVerificationFailed:
      msg1 = "ファイルの検証に失敗しました";
      break;
    case ota::Error::kWatchdogNow:
      msg1 = "ダウンロードが停止したため";
      msg2 = "再起動します";
      break;
    case ota::Error::kWatchdog10Sec:
      msg1 = "ダウンロードが進まなければ";
      msg2 = "10秒以内に再起動します";
      break;
    case ota::Error::kWatchdog30Sec:
      msg1 = "ダウンロードが進まなければ";
      msg2 = "30秒以内に再起動します";
      break;
    case ota::Error::kWatchdog60Sec:
      msg1 = "ダウンロードが進まなければ";
      msg2 = "60秒以内に再起動します";
      break;
    case ota::Error::kWatchdog180Sec:
      msg1 = "ダウンロードが進まなければ";
      msg2 = "3分以内に再起動します";
      break;
  }
  M5.Lcd.setFont(kFontJaMB);
  const int y = kHeaderHeight + kOtaProgressY + kOtaProgressHeight + kMargin;
  const int lh = M5.Lcd.fontHeight();
  M5.Lcd.fillRect(0, y, kScreenWidth, lh * 2, TFT_WHITE);
  M5.Lcd.setTextColor(kStatusDanger);
  M5.Lcd.setTextDatum(TC_DATUM);
  M5.Lcd.drawString(msg1, kScreenWidth / 2, y);
  M5.Lcd.drawString(msg2, kScreenWidth / 2, y + M5.Lcd.fontHeight());
}

void DrawClock() {
  const kb::LockGuard lock(g_mutex);
  M5.Lcd.setFont(kFontS);
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setCursor(8, 3);

  struct tm timeinfo {};
  if (!getLocalTime(&timeinfo, 10)) {
    M5.Lcd.setTextColor(kKachakaGray5, TFT_WHITE);
    M5.Lcd.print("??:??");
    return;
  }

  M5.Lcd.setTextColor(kKachakaBlack, TFT_WHITE);
  M5.Lcd.printf("%2d:%02d ", timeinfo.tm_hour, timeinfo.tm_min);
}

void DrawWiFiSignalStrength(const bool connected, const int rssi) {
  const kb::LockGuard lock(g_mutex);
  int siglevel = 0;
  if (rssi <= -90) {
    siglevel = 0;
  } else if (rssi <= -80) {
    siglevel = 1;
  } else if (rssi <= -70) {
    siglevel = 2;
  } else if (rssi <= -67) {
    siglevel = 3;
  } else {
    siglevel = 4;
  }
  constexpr int kBarWidth = 3;
  constexpr int kBarPadding = 2;
  for (int i = 0; i < 4; i++) {
    M5.Lcd.fillRect(kWiFiSignalStrengthX + (kBarWidth + kBarPadding) * i,
                    kHeaderMargin + (3 - i) * 3, kBarWidth, 5 + 3 * i,
                    !connected            ? TFT_RED
                    : siglevel >= (i + 1) ? kKachakaGray5
                                          : kKachakaGray2);
  }
}

void DrawButtonUsage(const int id, const char* text) {
  const int width = M5.Lcd.width();
  const int height = M5.Lcd.height();
  if (id < 1 || 3 < id) {
    return;
  }
  const int x = id == 1 ? width / 5 : id == 2 ? width / 2 : width * 4 / 5;
  const int s = M5.Lcd.fontHeight();
  if (text != nullptr) {
    const int w = M5.Lcd.textWidth(text);
    M5.Lcd.setCursor(std::max(0, x - w / 2), height - s * 2);
    M5.Lcd.print(text);
  }
  const auto dark = M5.Lcd.color565(32, 32, 32);
  const auto light = M5.Lcd.color565(192, 192, 192);
  M5.Lcd.fillTriangle(x - s / 2, height - s, x + s / 2, height - s, x, height,
                      text != nullptr ? light : dark);
}

}  // namespace screen
