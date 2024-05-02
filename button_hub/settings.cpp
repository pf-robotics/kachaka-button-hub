#include "settings.hpp"

#include "common.hpp"

static constexpr char kDefaultWiFiSsid[] = "";
static constexpr char kDefaultWiFiPassword[] = "";
static constexpr char kDefaultRobotHost[] = "";
static constexpr char kDefaultNtpServer[] = "time.aws.com";
static constexpr int kDefaultBeepVolume = 5;
static constexpr int kDefaultScreenBrightness = 64;
static constexpr bool kDefaultAutoOtaIsEnabled = false;
static constexpr bool kDefaultAutoRefetchOnUiLoad = false;

Settings::Settings() : prefs_(nullptr) {}

void Settings::Begin(Preferences* prefs) {
  prefs_ = prefs;
  wifi_ssid_ = prefs_->getString("wifi_ssid", kDefaultWiFiSsid);
  wifi_pass_ = prefs_->getString("wifi_pass", kDefaultWiFiPassword);
  robot_host_ = prefs_->getString("api_host", kDefaultRobotHost);
  ntp_server_ = prefs_->getString("ntp_server", kDefaultNtpServer);
  beep_volume_ = prefs_->getInt("beep_volume", kDefaultBeepVolume);
  screen_brightness_ =
      prefs_->getInt("scrn_brightness", kDefaultScreenBrightness);
  auto_ota_is_enabled_ = prefs_->getBool("auto_ota", kDefaultAutoOtaIsEnabled);
  auto_refetch_on_ui_load_ =
      prefs_->getBool("auto_refetch", kDefaultAutoRefetchOnUiLoad);

  Serial.printf(
      "Settings: ssid=\"%s\", pass=XXXX, host=\"%s\", ntp=\"%s\", beep=%d, "
      "brightness=%d, auto_ota=%d, auto_refetch\n",
      wifi_ssid_.c_str(), robot_host_.c_str(), ntp_server_.c_str(),
      beep_volume_, screen_brightness_, auto_ota_is_enabled_,
      auto_refetch_on_ui_load_);
  Serial.printf(
      "OTA settings: ota_endpoint=\"%s\", ota_label=\"%s\", "
      "reboot_ota_url=\"%s\"\n",
      GetOtaEndpoint(), GetOtaLabel(), GetOtaUrlAfterBoot().c_str());
  Serial.printf("Values: next_id=%d, ota_fail=%d\n", GetNextButtonId(),
                GetOtaFailCount());
}

void Settings::Reset() {
  prefs_->clear();
  Begin(prefs_);
}

const String& Settings::GetWiFiSsid() const {
  Check();
  return wifi_ssid_;
}

const String& Settings::GetWiFiPass() const {
  Check();
  return wifi_pass_;
}

const String& Settings::GetRobotHost() const {
  Check();
  return robot_host_;
}

const String& Settings::GetNtpServer() const {
  Check();
  return ntp_server_;
}

int Settings::GetBeepVolume() const {
  Check();
  return beep_volume_;
}

int Settings::GetScreenBrightness() const {
  Check();
  return screen_brightness_;
}

bool Settings::GetAutoOtaIsEnabled() const {
  Check();
  return auto_ota_is_enabled_;
}

bool Settings::GetAutoRefetchOnUiLoad() const {
  Check();
  return auto_refetch_on_ui_load_;
}

const char* Settings::GetOtaEndpoint() const {
#ifdef OTA_ENDPOINT
  return OTA_ENDPOINT;
#else
  return "";
#endif
}

const char* Settings::GetOtaLabel() const {
#ifdef OTA_LABEL
  return OTA_LABEL;
#else
  return "";
#endif
}

void Settings::SetWiFiSsid(const String& ssid) {
  Check();
  wifi_ssid_ = ssid;
  prefs_->putString("wifi_ssid", ssid);
}

void Settings::SetWiFiPass(const String& pass) {
  Check();
  wifi_pass_ = pass;
  prefs_->putString("wifi_pass", pass);
}

void Settings::SetRobotHost(const String& host) {
  Check();
  robot_host_ = host;
  prefs_->putString("api_host", host);
}

void Settings::SetNtpServer(const String& host) {
  Check();
  ntp_server_ = host;
  prefs_->putString("ntp_server", host);
}

void Settings::SetBeepVolume(const int volume) {
  Check();
  beep_volume_ = volume;
  prefs_->putInt("beep_volume", volume);
}

void Settings::SetScreenBrightness(const int brightness) {
  Check();
  screen_brightness_ = brightness;
  prefs_->putInt("scrn_brightness", brightness);
}

void Settings::SetAutoOtaIsEnabled(const bool enable) {
  Check();
  auto_ota_is_enabled_ = enable;
  prefs_->putBool("auto_ota", enable);
}

void Settings::SetAutoRefetchOnUiLoad(const bool enable) {
  Check();
  auto_refetch_on_ui_load_ = enable;
  prefs_->putBool("auto_refetch", enable);
}

int Settings::GetNextButtonId() {
  Check();
  const int next_id = prefs_->getInt("next_button_id", 1);
  prefs_->putInt("next_button_id", next_id + 1);
  return next_id;
}

int Settings::IncrementOtaFailCount() {
  Check();
  const int count = GetOtaFailCount() + 1;
  prefs_->putInt("ota_fail_count", count);
  return count;
}

void Settings::ClearOtaFailCount() {
  Check();
  prefs_->putInt("ota_fail_count", 0);
}

int Settings::GetOtaFailCount() {
  Check();
  return prefs_->getInt("ota_fail_count", 0);
}

String Settings::GetOtaUrlAfterBoot() {
  Check();
  return prefs_->getString("reboot_ota_url");
}

void Settings::SetOtaUrlAfterBoot(const String& url) {
  Check();
  prefs_->putString("reboot_ota_url", url);
}

void Settings::Check() const {
  if (prefs_ == nullptr) {
    Serial.println("Settings::Begin() should be called first");
    ESP.restart();
  }
}

Settings g_settings;
