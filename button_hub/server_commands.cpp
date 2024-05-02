#include "server_commands.hpp"

#include <ArduinoJson.h>
#include <vector>

#include "command_table.hpp"
#include "command_table_io.hpp"
#include "common.hpp"
#include "from_json.hpp"
#include "server.hpp"
#include "to_json.hpp"

void HandleGetObservedButtons(AsyncWebServerRequest* request,
                              CommandTable& command_table) {
  const String json = to_json::ConvertObservedButtons(
      command_table.GetObservedButtons(), command_table.GetButtonNames());
  request->send(200, "text/json; charset=utf-8", json);
}

void HandlePostCommand(AsyncWebServerRequest* request, const String& body,
                       CommandTable& command_table) {
  if (command_table_io::LoadCommand(body, command_table)) {
    command_table.Save();
    // button names may have changed if the button is new
    server::SendToWs(to_json::ConvertObservedButtons(
        command_table.GetObservedButtons(), command_table.GetButtonNames()));
    server::SendToWs(to_json::ConvertCommands(command_table.GetCommands()));
    request->send(200, "text/plain", "OK");
  } else {
    request->send(400, "text/plain", "Bad Request");
  }
}

void HandlePutCommands(AsyncWebServerRequest* request, const String& body,
                       CommandTable& command_table) {
  if (command_table_io::LoadCommandArray(body, command_table)) {
    command_table.Save();
    server::SendToWs(to_json::ConvertCommands(command_table.GetCommands()));
    request->send(200, "text/plain", "OK");
  } else {
    request->send(400, "text/plain", "Bad Request");
  }
}

void HandleGetCommands(AsyncWebServerRequest* request,
                       CommandTable& command_table) {
  String json = to_json::ConvertCommands(command_table.GetCommands());
  request->send(200, "text/json; charset=utf-8", json);
}

void HandleDeleteCommand(AsyncWebServerRequest* request, const String& body,
                         CommandTable& command_table) {
  // {
  //   "button": {
  //     "apple_i_beacon": {
  //       "address": "00:00:00:00:00:00",
  //       "uuid": "00000000-0000-0000-0000-000000000000",
  //       "major": 0,
  //       "minor": 0,
  //     },
  //   },
  // }
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const JsonObject& root = doc.as<JsonObject>();
  if (!root.containsKey("button")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  KButton button{};
  const JsonObject& button_json = root["button"];
  const bool ok = from_json::ConvertButtonJson(button_json, button);
  if (!ok) {
    request->send(400, "text/plain", "Bad Request");
    return;
  }

  command_table.DeleteCommand(button);
  command_table.Save();
  server::SendToWs(to_json::ConvertCommands(command_table.GetCommands()));
  server::SendToWs(to_json::ConvertObservedButtons(
      command_table.GetObservedButtons(), command_table.GetButtonNames()));

  request->send(200, "text/plain", "OK");
}

void HandleSetButtonName(AsyncWebServerRequest* request, const String& body,
                         CommandTable& command_table) {
  // {
  //   "button": {
  //     "apple_i_beacon": {
  //       "address": "00:00:00:00:00:00",
  //       "uuid": "00000000-0000-0000-0000-000000000000",
  //       "major": 0,
  //       "minor": 0,
  //     },
  //   },
  //   "name": "Button 1"
  // }
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const JsonObject& root = doc.as<JsonObject>();
  if (!root.containsKey("button") || !root.containsKey("name")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  KButton button{};
  const JsonObject& button_json = root["button"];
  const bool ok = from_json::ConvertButtonJson(button_json, button);
  if (!ok) {
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const String& name = root["name"].as<String>();

  command_table.SetButtonName(button, name);
  command_table.Save();
  server::SendToWs(to_json::ConvertObservedButtons(
      command_table.GetObservedButtons(), command_table.GetButtonNames()));

  request->send(200, "text/plain", "OK");
}

void HandleDeleteButtonName(AsyncWebServerRequest* request, const String& body,
                            CommandTable& command_table) {
  // {
  //   "button": {
  //     "apple_i_beacon": {
  //       "address": "00:00:00:00:00:00",
  //       "uuid": "00000000-0000-0000-0000-000000000000",
  //       "major": 0,
  //       "minor": 0,
  //     },
  //   },
  // }
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  const JsonObject& root = doc.as<JsonObject>();
  if (!root.containsKey("button")) {
    Serial.println("ERROR: Invalid JSON");
    request->send(400, "text/plain", "Bad Request");
    return;
  }
  KButton button{};
  const JsonObject& button_json = root["button"];
  const bool ok = from_json::ConvertButtonJson(button_json, button);
  if (!ok) {
    request->send(400, "text/plain", "Bad Request");
    return;
  }

  command_table.DeleteButtonName(button);
  command_table.Save();
  server::SendToWs(to_json::ConvertObservedButtons(
      command_table.GetObservedButtons(), command_table.GetButtonNames()));

  request->send(200, "text/plain", "OK");
}
