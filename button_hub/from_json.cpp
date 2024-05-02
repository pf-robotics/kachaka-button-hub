#include "from_json.hpp"

namespace from_json {

bool ConvertCommandJson(JsonObject& root, KButton& out_button,
                        Command& out_command) {
  // {
  //   "button": {
  //     "apple_i_beacon": {
  //       "address": "00:00:00:00:00:00",
  //       "uuid": "00000000-0000-0000-0000-000000000000",
  //       "major": 0,
  //       "minor": 0,
  //     },
  //   },
  //   "command": {
  //     "type": 1,
  //     "move_shelf": {
  //       "shelf_id": "shelf_id",
  //       "location_id": "location_id"
  //     },
  //     "cancel_all": true,
  //     "tts_on_success": "thank you",
  //     "deferrable": false,
  //     "lock_duration_sec": 10.0
  //   }
  // }
  if (!root.containsKey("button") || !root.containsKey("command")) {
    Serial.println("ERROR: Invalid JSON");
    return false;
  }
  const JsonObject& button_json = root["button"];
  const bool ok = from_json::ConvertButtonJson(button_json, out_button);
  if (!ok) {
    return false;
  }
  const JsonObject& command = root["command"];
  const int type = command["type"].as<int>();
  const bool cancel_all = command["cancel_all"].as<bool>();
  String tts_on_success = (command.containsKey("tts_on_success") &&
                           command["tts_on_success"].is<String>())
                              ? command["tts_on_success"].as<String>()
                              : "";
  const bool deferrable =
      (command.containsKey("deferrable") ? command["deferrable"].as<bool>()
                                         : false);
  const double lock_duration_sec =
      command.containsKey("lock_duration_sec")
          ? command["lock_duration_sec"].as<double>()
          : 0.0;

  switch (type) {
    case static_cast<int>(CommandType::MOVE_SHELF): {
      if (!command.containsKey("move_shelf")) {
        Serial.println("ERROR: Invalid JSON");
        return false;
      }
      const JsonObject& move_shelf = command["move_shelf"];
      if (!move_shelf.containsKey("shelf_id") ||
          !move_shelf.containsKey("location_id")) {
        Serial.println("ERROR: Invalid JSON");
        return false;
      }
      out_command.type = CommandType::MOVE_SHELF;
      out_command.move_shelf = {move_shelf["shelf_id"],
                                move_shelf["location_id"], false};
      out_command.cancel_all = cancel_all;
      out_command.tts_on_success = std::move(tts_on_success);
      out_command.deferrable = deferrable;
      out_command.lock_duration_sec = lock_duration_sec;
      break;
    }
    case static_cast<int>(CommandType::RETURN_SHELF): {
      if (!command.containsKey("return_shelf")) {
        Serial.println("ERROR: Invalid JSON");
        return false;
      }
      const JsonObject& return_shelf = command["return_shelf"];
      if (!return_shelf.containsKey("shelf_id")) {
        Serial.println("ERROR: Invalid JSON");
        return false;
      }
      out_command.type = CommandType::RETURN_SHELF;
      out_command.return_shelf = {return_shelf["shelf_id"]};
      out_command.cancel_all = cancel_all;
      out_command.tts_on_success = std::move(tts_on_success);
      out_command.deferrable = deferrable;
      out_command.lock_duration_sec = lock_duration_sec;
      break;
    }
    case static_cast<int>(CommandType::MOVE_TO_LOCATION): {
      if (!command.containsKey("move_to_location")) {
        Serial.println("ERROR: Invalid JSON");
        return false;
      }
      const JsonObject& move_to_location = command["move_to_location"];
      if (!move_to_location.containsKey("location_id")) {
        Serial.println("ERROR: Invalid JSON");
        return false;
      }
      out_command.type = CommandType::MOVE_TO_LOCATION;
      out_command.move_to_location = {move_to_location["location_id"]};
      out_command.cancel_all = cancel_all;
      out_command.tts_on_success = std::move(tts_on_success);
      out_command.deferrable = deferrable;
      out_command.lock_duration_sec = lock_duration_sec;
      break;
    }
    case static_cast<int>(CommandType::RETURN_HOME): {
      out_command.type = CommandType::RETURN_HOME;
      out_command.cancel_all = cancel_all;
      out_command.tts_on_success = std::move(tts_on_success);
      out_command.deferrable = deferrable;
      out_command.lock_duration_sec = lock_duration_sec;
      break;
    }
    case static_cast<int>(CommandType::SPEAK): {
      if (!command.containsKey("speak")) {
        Serial.println("ERROR: Invalid JSON");
        return false;
      }
      const JsonObject& speak = command["speak"];
      if (!speak.containsKey("text")) {
        Serial.println("ERROR: Invalid JSON");
        return false;
      }
      out_command.type = CommandType::SPEAK;
      out_command.speak = {speak["text"]};
      out_command.cancel_all = cancel_all;
      out_command.tts_on_success = std::move(tts_on_success);
      out_command.deferrable = deferrable;
      out_command.lock_duration_sec = lock_duration_sec;
      break;
    }
    case static_cast<int>(CommandType::PROCEED): {
      out_command.type = CommandType::PROCEED;
      out_command.cancel_all = false;
      out_command.tts_on_success.clear();
      out_command.deferrable = false;
      out_command.lock_duration_sec = 0.0;
      break;
    }
    case static_cast<int>(CommandType::CANCEL_COMMAND): {
      out_command.type = CommandType::CANCEL_COMMAND;
      out_command.cancel_all = false;
      out_command.tts_on_success.clear();
      out_command.deferrable = false;
      out_command.lock_duration_sec = 0.0;
      break;
    }
  }
  return true;
}

bool ConvertButtonJson(JsonObject json, KButton& out) {
  if (json.containsKey("apple_i_beacon")) {
    const JsonObject& beacon_json = json["apple_i_beacon"];
    if (!beacon_json.containsKey("address") ||
        !beacon_json.containsKey("uuid") || !beacon_json.containsKey("major") ||
        !beacon_json.containsKey("minor")) {
      Serial.println("ERROR: Invalid JSON");
      return false;
    }
    const String& address = beacon_json["address"].as<String>();
    const String& uuid = beacon_json["uuid"].as<String>();
    const uint16_t major = beacon_json["major"].as<uint16_t>();
    const uint16_t minor = beacon_json["minor"].as<uint16_t>();
    AppleIBeacon beacon;
    DeserializeAddressToString(address.c_str(), address.length(),
                               beacon.address);
    DeserializeUuidToString(uuid.c_str(), uuid.length(), beacon.uuid);
    beacon.major = major;
    beacon.minor = minor;
    out = KButton(beacon);
    return true;
  }
  if (json.containsKey("m5_button")) {
    const JsonObject& m5_json = json["m5_button"];
    if (!m5_json.containsKey("id")) {
      Serial.println("ERROR: Invalid JSON");
      return false;
    }
    const int id = m5_json["id"].as<int>();
    M5Button m5_button(id);
    out = KButton(m5_button);
    return true;
  }
  Serial.println("ERROR: Invalid JSON");
  return false;
}

}  // namespace from_json
