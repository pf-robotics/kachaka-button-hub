#pragma once

#include <deque>
#include <map>

#include "command_table.hpp"
#include "common.hpp"
#include "settings.hpp"
#include "types.hpp"

namespace to_json {

String ConvertHubInfo(const int client_count);
String ConvertRobotInfo(const RobotInfoHolder& robot_info);
String ConvertSettings(const Settings& settings);

String ConvertObservedButtons(
    const std::deque<ObservedButton>& observed_buttons,
    const std::map<KButton, String>& button_names);
String ConvertCommands(const std::map<KButton, Command>& commands);

}  // namespace to_json
