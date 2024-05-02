#pragma once

#include "ota.hpp"

namespace screen {

void Begin(int brightness);

void DrawMainPage(const char* ssid, const char* hub_host,
                  const char* robot_host);
void DrawStatusInMainPage(bool fetching, bool has_robot_version,
                          bool has_shelves, bool has_locations,
                          bool need_redraw);
void DrawPingResult(bool ok, float time_ms, int ng_count);
void DrawCommandSent(bool sent);

void DrawSettingPage(const char* hub_host);

void DrawOtaPage();
void DrawOtaProgress(double percent);
void DrawOtaError(ota::Error error_code);

void DrawClock();
void DrawWiFiSignalStrength(bool connected, int rssi);

void DrawButtonUsage(int id, const char* text);  // id: 1, 2, 3

}  // namespace screen
