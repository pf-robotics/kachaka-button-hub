#pragma once

#include <ESPAsyncWebServer.h>
#include <M5Unified.h>

#include "command_table.hpp"
#include "types.hpp"

namespace server {

void SetupHttpServerForWiFiSetting(RobotInfoHolder& robot_info,
                                   CommandTable& command_table);
void SetupHttpServer(RobotInfoHolder& robot_info, CommandTable& command_table);

void EnqueueWsMessage(String msg);
void FlushWsMessageQueue();

// True while a WebSocket client is connected or an HTTP request was handled
// within the last 3 minutes.
bool IsUiActive();

void Stop();

}  // namespace server
