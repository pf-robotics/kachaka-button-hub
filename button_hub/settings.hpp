#pragma once

#include <Preferences.h>

class Settings {
 public:
  Settings();

  Settings(const Settings&) = delete;
  Settings& operator=(const Settings&) = delete;

  void Begin(Preferences* prefs);
  void Reset();

  const String& GetWiFiSsid() const;
  const String& GetWiFiPass() const;
  const String& GetNetworkIpAddress() const;
  const String& GetNetworkSubnetMask() const;
  const String& GetNetworkGateway() const;
  const String& GetNetworkDnsServer1() const;
  const String& GetNetworkDnsServer2() const;

  const String& GetRobotHost() const;
  const String& GetNtpServer() const;
  int GetBeepVolume() const;
  int GetScreenBrightness() const;
  bool GetAutoOtaIsEnabled() const;
  bool GetOneShotAutoOtaIsEnabled() const;
  bool GetAutoRefetchOnUiLoad() const;
  bool GetGpioButtonIsEnabled() const;

  const char* GetOtaEndpoint() const;
  const char* GetOtaLabel() const;

  void SetWiFiSsid(const String& ssid);
  void SetWiFiPass(const String& pass);
  void SetNetworkIpAddress(const String& ip);
  void SetNetworkSubnetMask(const String& netmask);
  void SetNetworkGateway(const String& gw);
  void SetNetworkDnsServer1(const String& dns1);
  void SetNetworkDnsServer2(const String& dns2);
  void SetRobotHost(const String& host);
  void SetNtpServer(const String& host);
  void SetBeepVolume(int volume);            // 0-11
  void SetScreenBrightness(int brightness);  // 0-255
  void SetAutoOtaIsEnabled(bool enable);
  void SetOneShotAutoOtaIsEnabled(bool enable);
  void SetAutoRefetchOnUiLoad(bool enable);
  void SetGpioButtonIsEnabled(bool enable);

  // Non-settings
  int GetNextButtonId();
  int GetNextLoggingId();
  int IncrementOtaFailCount();
  void ClearOtaFailCount();
  int GetOtaFailCount();
  String GetOtaUrlAfterBoot();
  void SetOtaUrlAfterBoot(const String& url);

 private:
  void Check() const;

  Preferences* prefs_;
  String wifi_ssid_;
  String wifi_pass_;
  String net_ip_address_;
  String net_subnet_mask_;
  String net_gateway_;
  String net_dns_server_1_;
  String net_dns_server_2_;
  String robot_host_;
  String ntp_server_;
  int beep_volume_;
  int screen_brightness_;
  bool auto_ota_is_enabled_;
  bool one_shot_auto_ota_is_enabled_;
  bool auto_refetch_on_ui_load_;
  bool gpio_button_is_enabled_;
};

extern Settings g_settings;
