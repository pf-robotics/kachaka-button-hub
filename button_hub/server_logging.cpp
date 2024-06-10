#include "server_logging.hpp"

#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "ESPAsyncWebServer.h"

namespace server {

void HandleLoggingList(AsyncWebServerRequest* request) {
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("ERROR: Failed to open directory");
    request->send(500, "text/plain", "Internal Server Error");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("ERROR: Not a directory");
    request->send(500, "text/plain", "Internal Server Error");
    return;
  }

  {
    JsonDocument doc;
    doc["total_bytes"] = SPIFFS.totalBytes();
    doc["used_bytes"] = SPIFFS.usedBytes();
    JsonObject files = doc.createNestedObject("files");
    {
      File entry = root.openNextFile();
      while (entry) {
        String name = entry.name();
        if (name.startsWith("log")) {
          files[name] = entry.size();
        }
        entry = root.openNextFile();
      }
    }
    String out;
    serializeJson(doc, out);
    request->send(200, "text/plain; charset=UTF-8", out);
  }
}

void HandleLoggingGet(AsyncWebServerRequest* request, const String& path,
                      const bool download) {
  Serial.printf("Retrieving log path=\"%s\" (download=%s)\n", path.c_str(),
                download ? "true" : "false");
  if (!SPIFFS.exists(path)) {
    request->send(404, "text/plain", "Not Found");
    return;
  }
  File file = SPIFFS.open(path, "r");
  if (!file) {
    request->send(500, "text/plain", "Internal Server Error");
    return;
  }
  request->send(new AsyncFileResponse(std::move(file), path,
                                      "text/plain; charset=UTF-8", download));
}

void HandleLoggingDelete(AsyncWebServerRequest* request, const String& path) {
  Serial.printf("Deleting log path=\"%s\"\n", path.c_str());
  if (!SPIFFS.exists(path)) {
    request->send(404, "text/plain", "Not Found");
    return;
  }
  const bool ok = SPIFFS.remove(path);
  if (!ok) {
    request->send(500, "text/plain", "Internal Server Error");
    return;
  }
  request->send(200, "text/plain", "OK");
}

}  // namespace server
