#include "command_table_io.hpp"

#include <ArduinoJson.h>

#include "from_json.hpp"

namespace command_table_io {

bool LoadCommand(const String& json, CommandTable& command_table) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    return false;
  }
  JsonObject root = doc.as<JsonObject>();
  KButton button;
  Command command;
  if (!from_json::ConvertCommandJson(root, button, command)) {
    return false;
  }
  command_table.SetCommand(button, command);
  return true;
}

bool LoadCommandArray(const String& json, CommandTable& command_table) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    return false;
  }
  JsonObject root = doc.as<JsonObject>();
  if (!root.containsKey("commands")) {
    Serial.println("ERROR: Failed to parse JSON");
    return false;
  }

  command_table.DeleteAllCommands();

  bool ok = true;
  JsonArray arr = root["commands"].as<JsonArray>();
  for (JsonObject obj : arr) {
    KButton button;
    Command command;
    if (!from_json::ConvertCommandJson(obj, button, command)) {
      ok = false;
      continue;
    }
    command_table.SetCommand(button, command);
  }
  return ok;
}

bool LoadButtonNameArray(const String& json, CommandTable& command_table) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("ERROR: Failed to parse JSON");
    return false;
  }
  JsonObject root = doc.as<JsonObject>();
  if (!root.containsKey("buttons")) {
    Serial.println("ERROR: Failed to parse JSON");
    return false;
  }

  command_table.DeleteAllButtonNames();

  for (JsonObject item : root["buttons"].as<JsonArray>()) {
    if (!item.containsKey("name")) {
      continue;
    }
    KButton button;
    if (!from_json::ConvertButtonJson(item, button)) {
      continue;
    }
    command_table.SetButtonName(button, item["name"].as<String>());
  }
  return true;
}

}  // namespace command_table_io
