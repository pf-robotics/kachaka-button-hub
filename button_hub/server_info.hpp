#pragma once

#include <ESPAsyncWebServer.h>

#include "command_table.hpp"
#include "types.hpp"

void HandleGetRobotHost(AsyncWebServerRequest* request);
void HandleSetRobotHost(AsyncWebServerRequest* request, const String& body);
void HandleGetWiFi(AsyncWebServerRequest* request);
void HandleSetWiFi(AsyncWebServerRequest* request, const String& body);
void HandleGetBeepVolume(AsyncWebServerRequest* request);
void HandleSetBeepVolume(AsyncWebServerRequest* request, const String& body);
void HandleGetScreenBrightness(AsyncWebServerRequest* request);
void HandleSetScreenBrightness(AsyncWebServerRequest* request,
                               const String& body);
void HandleGetAutoOtaIsEnabled(AsyncWebServerRequest* request);
void HandleSetAutoOtaIsEnabled(AsyncWebServerRequest* request,
                               const String& body);
void HandleGetAutoRefetchOnUiLoad(AsyncWebServerRequest* request);
void HandleSetAutoRefetchOnUiLoad(AsyncWebServerRequest* request,
                                  const String& body);

void HandleGetDesiredHubVersion(AsyncWebServerRequest* request);
void HandleGetOtaImageUrlByVersion(AsyncWebServerRequest* request,
                                   const String& body);
void HandleStartAutoOta(AsyncWebServerRequest* request);
void HandleOtaByImageUrl(AsyncWebServerRequest* request, const String& body);

void HandleClearAllData(AsyncWebServerRequest* krequest,
                        CommandTable& command_table);
