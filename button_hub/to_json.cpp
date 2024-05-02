#include "to_json.hpp"

#include <ArduinoJson.h>

#include "settings.hpp"
#include "types.hpp"
#include "version.hpp"

namespace to_json {

String ConvertHubInfo(const int client_count) {
  // {
  //   "type": "hub_info",
  //   "hub_version": "1.2.3",
  //   "ota_label": "",
  // }
  const char* ota_endpoint = g_settings.GetOtaEndpoint();
  JsonDocument doc;
  doc["type"] = "hub_info";
  doc["hub_version"] = kVersion;
  doc["ota_available"] = ota_endpoint != nullptr && ota_endpoint[0] != '\0';
  doc["ota_label"] = g_settings.GetOtaLabel();
  doc["client_count"] = client_count;

  String out;
  serializeJson(doc, out);
  return out;
}

String ConvertRobotInfo(const RobotInfoHolder& robot_info) {
  // {
  //   "type": "robot_info",
  //   "robot_version": "1.0.0",
  //   "shelves": [
  //     {
  //       "id": "shelf1",
  //       "name": "Shelf 1"
  //     },
  //     {
  //       ...
  //     }
  //   ],
  //   "locations": [
  //     {
  //       "id": "location1",
  //       "name": "Location 1"
  //     },
  //     {
  //       ...
  //     }
  //   ]
  // }
  JsonDocument doc;
  doc["type"] = "robot_info";
  if (robot_info.has_robot_version) {
    doc["robot_version"] = robot_info.robot_version;
  }
  if (robot_info.has_shelves) {
    doc["shelves"] = JsonArray();
    for (const Shelf& shelf : robot_info.shelves) {
      JsonObject shelf_json = doc["shelves"].createNestedObject();
      shelf_json["id"] = shelf.id;
      shelf_json["name"] = shelf.name;
    }
  }
  if (robot_info.has_locations) {
    doc["locations"] = JsonArray();
    for (const Location& location : robot_info.locations) {
      JsonObject location_json = doc["locations"].createNestedObject();
      location_json["id"] = location.id;
      location_json["name"] = location.name;
    }
  }

  String out;
  serializeJson(doc, out);
  return out;
}

String ConvertSettings(const Settings& settings) {
  // {
  //   "type": "settings",
  //   "settings": {
  //     "wifi_ssid": "",
  //     "wifi_pass": "",
  //     "robot_host": "",
  //     "ntp_server": "",
  //     "beep_volume": 0,
  //     "screen_brightness": 0,
  //     "auto_ota_is_enabled": 0,
  //     "auto_refetch_on_ui_load": 0,
  //   }
  // }
  JsonDocument doc;
  doc["type"] = "settings";
  JsonObject settings_json = doc.createNestedObject("settings");
  settings_json["wifi_ssid"] = settings.GetWiFiSsid();
  settings_json["wifi_pass"] = settings.GetWiFiPass();
  settings_json["robot_host"] = settings.GetRobotHost();
  settings_json["ntp_server"] = settings.GetNtpServer();
  settings_json["beep_volume"] = settings.GetBeepVolume();
  settings_json["screen_brightness"] = settings.GetScreenBrightness();
  settings_json["auto_ota_is_enabled"] = settings.GetAutoOtaIsEnabled();
  settings_json["auto_refetch_on_ui_load"] = settings.GetAutoRefetchOnUiLoad();

  String out;
  serializeJson(doc, out);
  return out;
}

static void FillButtonJson(const KButton& button, JsonObject object) {
  switch (button.type) {
    case ButtonType::kAppleIBeacon: {
      const AppleIBeacon& beacon = button.data.apple_i_beacon;
      JsonObject beacon_json = object.createNestedObject("apple_i_beacon");
      beacon_json["address"] = SerializeAddressToString(beacon.address);
      beacon_json["uuid"] = SerializeUuidToString(beacon.uuid);
      beacon_json["major"] = beacon.major;
      beacon_json["minor"] = beacon.minor;
    } break;
    case ButtonType::kM5Button: {
      const M5Button& m5_button = button.data.m5_button;
      JsonObject m5_button_json = object.createNestedObject("m5_button");
      m5_button_json["id"] = m5_button.id;
    } break;
  }
}

String ConvertObservedButtons(
    const std::deque<ObservedButton>& observed_buttons,
    const std::map<KButton, String>& button_names) {
  // {
  //   "type": "observed_buttons",
  //   "buttons": [
  //     {
  //       "timestamp": 10,
  //       "name": "Button 1",
  //       "apple_i_beacon": {
  //         "address": "00:00:00:00:00:00",
  //         "uuid": "00000000-0000-0000-0000-000000000000",
  //         "major": 0,
  //         "minor": 0,
  //       },
  //     },
  //     {
  //       ...
  //     }
  //   ],
  //   "timestamp_now": 10
  // }

  std::map<KButton, String> names = button_names;

  JsonDocument doc;
  doc["type"] = "observed_buttons";
  JsonArray buttons = doc.createNestedArray("buttons");

  // observed recently (has "timestamp" field)
  for (const auto& [timestamp, estimated_distance, button] : observed_buttons) {
    JsonObject item = buttons.createNestedObject();
    item["timestamp"] = timestamp;
    if (estimated_distance >= 0.0) {
      item["estimated_distance"] = estimated_distance;
    }
    const auto iter = names.find(button);
    if (iter != names.end()) {
      item["name"] = iter->second;
      names.erase(iter);
    }
    FillButtonJson(button, item);
  }

  // named buttons
  for (const auto& [button, name] : names) {
    JsonObject item = buttons.createNestedObject();
    item["name"] = name;
    FillButtonJson(button, item);
  }

  std::time_t now;
  std::time(&now);
  doc["timestamp_now"] = now;

  String out;
  serializeJson(doc, out);
  return out;
}

String ConvertCommands(const std::map<KButton, Command>& commands) {
  // {
  //   "type": "commands",
  //   "timestamp_now": 10,
  //   "commands": [
  //     {
  //       "button": {
  //         "apple_i_beacon": {
  //           "address": "00:00:00:00:00:00",
  //           "uuid": "00000000-0000-0000-0000-000000000000",
  //           "major": 0,
  //           "minor": 0,
  //         },
  //       },
  //       "command": {
  //         "type": 1,
  //         "move_shelf": {
  //           "shelf_id": "shelf_id",
  //           "location_id": "location_id"
  //         }
  //         "cancel_all": true,
  //         "tts_on_success": "thank you",
  //         "deferrable": false,
  //         "lock_duration_sec": 10.0
  //       }
  //     },
  //     {
  //        ...
  //     }
  //   ]
  // }
  JsonDocument doc;
  doc["type"] = "commands";
  JsonArray commands_json = doc.createNestedArray("commands");
  for (const auto& [button, command] : commands) {
    JsonObject item = commands_json.createNestedObject();
    JsonObject button_json = item.createNestedObject("button");
    FillButtonJson(button, button_json);
    JsonObject command_json = item.createNestedObject("command");
    command_json["type"] = static_cast<int>(command.type);
    command_json["cancel_all"] = command.cancel_all;
    if (!command.tts_on_success.isEmpty()) {
      command_json["tts_on_success"] = command.tts_on_success;
    }
    command_json["deferrable"] = command.deferrable;
    if (std::fabs(command.lock_duration_sec) > 0.001) {
      command_json["lock_duration_sec"] = command.lock_duration_sec;
    }
    switch (command.type) {
      case CommandType::MOVE_SHELF: {
        JsonObject move_shelf = command_json.createNestedObject("move_shelf");
        move_shelf["shelf_id"] = command.move_shelf.target_shelf_id;
        move_shelf["location_id"] = command.move_shelf.destination_location_id;
      } break;
      case CommandType::RETURN_SHELF: {
        JsonObject return_shelf =
            command_json.createNestedObject("return_shelf");
        return_shelf["shelf_id"] = command.return_shelf.target_shelf_id;
      } break;
      case CommandType::MOVE_TO_LOCATION: {
        JsonObject move_to_location =
            command_json.createNestedObject("move_to_location");
        move_to_location["location_id"] =
            command.move_to_location.target_location_id;
      } break;
      case CommandType::RETURN_HOME: {
      } break;
      case CommandType::SPEAK: {
        JsonObject speak = command_json.createNestedObject("speak");
        speak["text"] = command.speak.text;
      } break;
    }
  }

  std::time_t now;
  std::time(&now);
  doc["timestamp"] = now;

  String out;
  serializeJson(doc, out);
  return out;
}

}  // namespace to_json
