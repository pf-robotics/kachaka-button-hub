#pragma once

#include <ESPAsyncWebServer.h>

namespace server {

void HandleLoggingList(AsyncWebServerRequest* request);
void HandleLoggingGet(AsyncWebServerRequest* request, const String& path,
                      bool download);
void HandleLoggingDelete(AsyncWebServerRequest* request, const String& path);

}  // namespace server
