#pragma once

#include <ESPAsyncWebServer.h>

#include "command_table.hpp"
#include "common.hpp"
#include "types.hpp"

namespace server {

void SetupHttpServerForWiFiSetting();
void SetupHttpServer(RobotInfoHolder& robot_info, CommandTable& command_table);
void SendToWs(const String& msg);

void Stop();

}  // namespace server
