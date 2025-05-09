#include "settings.hpp"

#include <M5Unified.h>

static constexpr char kDefaultWiFiSsid[] = "";
static constexpr char kDefaultWiFiPassword[] = "";
static constexpr char kDefaultRobotHost[] = "";
static constexpr char kDefaultNtpServer[] = "time.aws.com";
static constexpr int kDefaultBeepVolume = 5;
static constexpr int kDefaultScreenBrightness = 64;
static constexpr bool kDefaultAutoOtaIsEnabled = true;
static constexpr bool kDefaultOneShotAutoOtaIsEnabled = false;
static constexpr bool kDefaultAutoRefetchOnUiLoad = false;
static constexpr bool kDefaultGpioButtonIsEnabled = false;

Settings::Settings() : prefs_(nullptr) {}

void Settings::Begin(Preferences* prefs) {
  prefs_ = prefs;
  wifi_ssid_ = prefs_->getString("wifi_ssid", kDefaultWiFiSsid);
  wifi_pass_ = prefs_->getString("wifi_pass", kDefaultWiFiPassword);
  net_ip_address_ = prefs_->getString("net_ip", "");
  net_subnet_mask_ = prefs_->getString("net_subnet", "");
  net_gateway_ = prefs_->getString("net_gw", "");
  net_dns_server_1_ = prefs_->getString("net_dns1", "");
  net_dns_server_2_ = prefs_->getString("net_dns2", "");
  robot_host_ = prefs_->getString("api_host", kDefaultRobotHost);
  ntp_server_ = prefs_->getString("ntp_server", kDefaultNtpServer);
  beep_volume_ = prefs_->getInt("beep_volume", kDefaultBeepVolume);
  screen_brightness_ =
      prefs_->getInt("scrn_brightness", kDefaultScreenBrightness);
  auto_ota_is_enabled_ = prefs_->getBool("auto_ota", kDefaultAutoOtaIsEnabled);
  one_shot_auto_ota_is_enabled_ =
      prefs_->getBool("1shot_auto_ota", kDefaultOneShotAutoOtaIsEnabled);
  auto_refetch_on_ui_load_ =
      prefs_->getBool("auto_refetch", kDefaultAutoRefetchOnUiLoad);
  gpio_button_is_enabled_ =
      prefs_->getBool("gpio_button", kDefaultGpioButtonIsEnabled);

  Serial.printf(
      "Network: ssid=\"%s\", pass=XXXX, ip=\"%s\", gw=\"%s\", "
      "dns1=\"%s\", dns2=\"%s\", ntp=\"%s\"\n",
      wifi_ssid_.c_str(), net_ip_address_.c_str(), net_gateway_.c_str(),
      net_dns_server_1_.c_str(), net_dns_server_2_.c_str(),
      ntp_server_.c_str());
  Serial.printf(
      "Settings: host=\"%s\", beep=%d, brightness=%d, auto_ota=%d, "
      "auto_refetch=%d, gpio_button=%d\n",
      robot_host_.c_str(), beep_volume_, screen_brightness_,
      auto_ota_is_enabled_, auto_refetch_on_ui_load_, gpio_button_is_enabled_);
  Serial.printf(
      "OTA settings: ota_endpoint=\"%s\", ota_label=\"%s\", "
      "reboot_ota_url=\"%s\", auto_ota=%d, one_shot_auto_ota=%d\n",
      GetOtaEndpoint(), GetOtaLabel(), GetOtaUrlAfterBoot().c_str(),
      GetAutoOtaIsEnabled(), GetOneShotAutoOtaIsEnabled());
  Serial.printf("Values: next_btn_id=%d, next_log_id=%d, ota_fail=%d\n",
                prefs_->getInt("next_button_id", -1),
                prefs_->getInt("next_log_id", -1), GetOtaFailCount());
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

const String& Settings::GetNetworkIpAddress() const {
  Check();
  return net_ip_address_;
}

const String& Settings::GetNetworkSubnetMask() const {
  Check();
  return net_subnet_mask_;
}

const String& Settings::GetNetworkGateway() const {
  Check();
  return net_gateway_;
}

const String& Settings::GetNetworkDnsServer1() const {
  Check();
  return net_dns_server_1_;
}

const String& Settings::GetNetworkDnsServer2() const {
  Check();
  return net_dns_server_2_;
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

bool Settings::GetOneShotAutoOtaIsEnabled() const {
  Check();
  return one_shot_auto_ota_is_enabled_;
}

bool Settings::GetAutoRefetchOnUiLoad() const {
  Check();
  return auto_refetch_on_ui_load_;
}

bool Settings::GetGpioButtonIsEnabled() const {
  Check();
  return gpio_button_is_enabled_;
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

void Settings::SetNetworkIpAddress(const String& ip) {
  Check();
  net_ip_address_ = ip;
  prefs_->putString("net_ip", ip);
}

void Settings::SetNetworkSubnetMask(const String& netmask) {
  Check();
  net_subnet_mask_ = netmask;
  prefs_->putString("net_subnet", netmask);
}

void Settings::SetNetworkGateway(const String& gw) {
  Check();
  net_gateway_ = gw;
  prefs_->putString("net_gw", gw);
}

void Settings::SetNetworkDnsServer1(const String& dns1) {
  Check();
  net_dns_server_1_ = dns1;
  prefs_->putString("net_dns1", dns1);
}

void Settings::SetNetworkDnsServer2(const String& dns2) {
  Check();
  net_dns_server_2_ = dns2;
  prefs_->putString("net_dns2", dns2);
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

void Settings::SetOneShotAutoOtaIsEnabled(const bool enable) {
  Check();
  one_shot_auto_ota_is_enabled_ = enable;
  prefs_->putBool("1shot_auto_ota", enable);
}

void Settings::SetAutoRefetchOnUiLoad(const bool enable) {
  Check();
  auto_refetch_on_ui_load_ = enable;
  prefs_->putBool("auto_refetch", enable);
}

void Settings::SetGpioButtonIsEnabled(const bool enable) {
  Check();
  gpio_button_is_enabled_ = enable;
  prefs_->putBool("gpio_button", enable);
}

int Settings::GetNextButtonId() {
  Check();
  const int next_id = prefs_->getInt("next_button_id", 1);
  prefs_->putInt("next_button_id", next_id + 1);
  return next_id;
}

int Settings::GetNextLoggingId() {
  Check();
  const int next_id = prefs_->getInt("next_log_id", 1);
  prefs_->putInt("next_log_id", next_id + 1);
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
