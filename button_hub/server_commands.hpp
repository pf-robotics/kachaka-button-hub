#pragma once

#include <ESPAsyncWebServer.h>

#include "command_table.hpp"

void HandleGetObservedButtons(AsyncWebServerRequest* request,
                              CommandTable& command_table);
void HandlePostCommand(AsyncWebServerRequest* request, const String& body,
                       CommandTable& command_table);
void HandlePutCommands(AsyncWebServerRequest* request, const String& body,
                       CommandTable& command_table);
void HandleGetCommands(AsyncWebServerRequest* request,
                       CommandTable& command_table);
void HandleDeleteCommand(AsyncWebServerRequest* request, const String& body,
                         CommandTable& command_table);
void HandleSetButtonName(AsyncWebServerRequest* request, const String& body,
                         CommandTable& command_table);
void HandleDeleteButtonName(AsyncWebServerRequest* request, const String& body,
                            CommandTable& command_table);
