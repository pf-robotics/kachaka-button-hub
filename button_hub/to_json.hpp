#pragma once

#include <M5Unified.h>
#include <deque>
#include <map>

#include "command_table.hpp"
#include "settings.hpp"
#include "types.hpp"
#include "wifi.hpp"

namespace to_json {

String ConvertHubInfo(const int client_count);
String ConvertRobotInfo(const RobotInfoHolder& robot_info);
String ConvertSettings(const Settings& settings);
String ConvertWiFiApList(bool scanning,
                         const std::vector<wifi::WiFiAp>& wifi_ap_list);

String ConvertObservedButtons(
    const std::deque<ObservedButton>& observed_buttons,
    const std::map<KButton, String>& button_names);
String ConvertCommand(const Command& command);
String ConvertCommands(const std::map<KButton, Command>& commands);

}  // namespace to_json
