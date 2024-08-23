#include "command_table.hpp"

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#include "from_json.hpp"
#include "logging.hpp"
#include "mutex.hpp"
#include "settings.hpp"
#include "to_json.hpp"

constexpr int kFileVersion = 7;
static constexpr char const* kCommandTablePath = "/command_table.dat";
static constexpr char const* kTemporaryCommandTablePath = "/command_table.tmp";

static bool WriteInt32(File& file, const int32_t value) {
  const size_t retv =
      file.write(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
  if (retv != sizeof(value)) {
    logging::Log("Failed to write int32_t: %d", retv);
    return false;
  }
  return true;
}

static int32_t ReadInt32(File& file) {
  int32_t value = 0;
  const size_t retv =
      file.read(reinterpret_cast<uint8_t*>(&value), sizeof(value));
  if (retv != sizeof(value)) {
    logging::Log("Failed to read int32_t: %d", retv);
  }
  return value;
}

static bool WriteString(File& file, const String& str) {
  const int32_t size = static_cast<int32_t>(str.length());
  if (!WriteInt32(file, size)) {
    return false;
  }
  const size_t retv =
      file.write(reinterpret_cast<const uint8_t*>(str.c_str()), size);
  if (retv != size) {
    logging::Log("Failed to write string: %d != %u", size, retv);
    return false;
  }
  return true;
}

static String ReadString(File& file) {
  int32_t size = ReadInt32(file);
  std::vector<char> buf(size + 1);
  const size_t retv = file.read(reinterpret_cast<uint8_t*>(buf.data()), size);
  if (retv != size) {
    logging::Log("Failed to read int32_t: %d", retv);
  }
  buf.at(size) = '\0';
  return String(buf.data());
}

bool SerializeAddressToString(const uint8_t address[6], char* out, int len) {
  const int required_size = 18;  // 17 characters + null terminator
  if (len < required_size) {
    return false;
  }
  const int written =
      std::snprintf(out, len, "%02x:%02x:%02x:%02x:%02x:%02x", address[0],
                    address[1], address[2], address[3], address[4], address[5]);
  return written > 0 && written < len;
}

String SerializeAddressToString(const uint8_t address[6]) {
  char buf[18];
  SerializeAddressToString(address, buf, sizeof(buf));
  return buf;
}

bool DeserializeAddressToString(const char* data, int len, uint8_t out[6]) {
  if (len != 17) {
    return false;
  }
  for (int i = 0; i < len; ++i) {
    if ((i + 1) % 3 == 0) {
      if (data[i] != ':') {
        return false;
      }
    } else {
      if (!((data[i] >= '0' && data[i] <= '9') ||
            (data[i] >= 'a' && data[i] <= 'f') ||
            (data[i] >= 'A' && data[i] <= 'F'))) {
        return false;
      }
    }
  }
  const int retv =
      std::sscanf(data, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &out[0],
                  &out[1], &out[2], &out[3], &out[4], &out[5]);
  return retv == 6;
}

bool SerializeUuidToString(const uint8_t uuid[16], char* out, int len) {
  const int required_size = 37;  // 36 characters + null terminator
  if (len < required_size) {
    return false;
  }
  const int written = std::snprintf(
      out, len,
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
      uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14],
      uuid[15]);

  return written > 0 && written < len;
}

String SerializeUuidToString(const uint8_t uuid[16]) {
  char buf[37];
  SerializeUuidToString(uuid, buf, sizeof(buf));
  return buf;
}

bool DeserializeUuidToString(const char* data, int len, uint8_t out[16]) {
  if (len != 36) {
    return false;
  }
  for (int i = 0; i < len; ++i) {
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (data[i] != '-') {
        return false;
      }
    } else {
      if (!((data[i] >= '0' && data[i] <= '9') ||
            (data[i] >= 'a' && data[i] <= 'f') ||
            (data[i] >= 'A' && data[i] <= 'F'))) {
        return false;
      }
    }
  }
  const int retv =
      std::sscanf(data,
                  "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-"
                  "%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
                  &out[0], &out[1], &out[2], &out[3], &out[4], &out[5], &out[6],
                  &out[7], &out[8], &out[9], &out[10], &out[11], &out[12],
                  &out[13], &out[14], &out[15]);
  return retv == 16;
}

CommandTable::CommandTable(const int max_observed_buttons)
    : max_observed_buttons_(max_observed_buttons),
      observed_buttons_(),
      registered_commands_(),
      button_names_() {}

void CommandTable::NotifyObservedButton(const KButton& button,
                                        const double estimated_distance) {
  const kb::LockGuard lock(mutex_);

  std::time_t now{};
  std::time(&now);

  // Remove entries which has the same button
  observed_buttons_.erase(
      std::remove_if(observed_buttons_.begin(), observed_buttons_.end(),
                     [&button](const ObservedButton& observed_button) {
                       return observed_button.button == button;
                     }),
      observed_buttons_.end());

  // Push the new entry
  observed_buttons_.push_front({now, estimated_distance, button});

  // Pop the oldest entry if the size exceeds the limit
  if (observed_buttons_.size() > max_observed_buttons_) {
    observed_buttons_.pop_back();
  }
}

const std::deque<ObservedButton>& CommandTable::GetObservedButtons() const {
  return observed_buttons_;
}

void CommandTable::SetCommand(const KButton& button, const Command& command) {
  const kb::LockGuard lock(mutex_);
  SetCommandLocked(button, command);
}

void CommandTable::DeleteCommand(const KButton& button) {
  const kb::LockGuard lock(mutex_);
  DeleteCommandLocked(button);
  if (button.type == ButtonType::kAppleIBeacon) {
    button_names_.erase(button);
  }
}

std::vector<ButtonCommandPair> CommandTable::GetCommands() const {
  const kb::LockGuard lock(mutex_);
  return registered_commands_;
}

bool CommandTable::GetCommandByButton(const KButton& button,
                                      Command* command) const {
  const kb::LockGuard lock(mutex_);
  for (const auto& pair : registered_commands_) {
    if (pair.button == button) {
      *command = pair.command;
      return true;
    }
  }
  return false;
}

void CommandTable::SetButtonName(const KButton& button, const String& name) {
  const kb::LockGuard lock(mutex_);
  SetButtonNameLocked(button, name);
}

void CommandTable::DeleteButtonName(const KButton& button) {
  const kb::LockGuard lock(mutex_);
  button_names_.erase(button);
}

std::map<KButton, String> CommandTable::GetButtonNames() const {
  const kb::LockGuard lock(mutex_);
  return button_names_;
}

bool CommandTable::LoadCommand(const String& json) {
  const kb::LockGuard lock(mutex_);

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
  SetCommandLocked(button, command);
  return true;
}

bool CommandTable::LoadCommandArray(const String& json) {
  const kb::LockGuard lock(mutex_);
  return LoadCommandArrayLocked(json);
}

bool CommandTable::LoadCommandArrayLocked(const String& json) {
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

  registered_commands_.clear();

  bool ok = true;
  JsonArray arr = root["commands"].as<JsonArray>();
  for (JsonObject obj : arr) {
    KButton button;
    Command command;
    if (!from_json::ConvertCommandJson(obj, button, command)) {
      ok = false;
      continue;
    }
    SetCommandLocked(button, command);
  }
  return ok;
}

bool CommandTable::LoadButtonNameArrayLocked(const String& json) {
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

  button_names_.clear();

  for (JsonObject item : root["buttons"].as<JsonArray>()) {
    if (!item.containsKey("name")) {
      continue;
    }
    KButton button;
    if (!from_json::ConvertButtonJson(item, button)) {
      continue;
    }
    SetButtonNameLocked(button, item["name"].as<String>());
  }
  return true;
}

void CommandTable::Save() {
  const kb::LockGuard lock(mutex_);

  logging::Log("Save command table.");
  {
    File file = SPIFFS.open(kTemporaryCommandTablePath, "w");
    if (!file) {
      logging::Log("Failed to open the command file");
      return;
    }
    if (!WriteInt32(file, kFileVersion) ||
        !WriteString(file, to_json::ConvertObservedButtons(observed_buttons_,
                                                           button_names_)) ||
        !WriteString(file, to_json::ConvertCommands(registered_commands_))) {
      logging::Log("Failed to write the command file");
      return;
    }
    file.flush();
  }
  if (!SPIFFS.remove(kCommandTablePath)) {
    logging::Log("Failed to remove the old file");
  }
  if (!SPIFFS.rename(kTemporaryCommandTablePath, kCommandTablePath)) {
    logging::Log(
        "Failed to swap the command file and the temporary file. The command "
        "file is lost.");
  }
  logging::Log("Saved the command table: %d buttons, %d commands",
               button_names_.size(), registered_commands_.size());
}

void CommandTable::Load() {
  const kb::LockGuard lock(mutex_);

  logging::Log("Load command table.");
  File file = SPIFFS.open(kCommandTablePath);
  if (!file) {
    logging::Log("No setting file");
    return;
  }

  const int32_t version = ReadInt32(file);
  logging::Log("File version = %d", version);
  if (version != kFileVersion) {
    logging::Log("Invalid version %d", version);
    return;
  }

  const String buttons_json = ReadString(file);
  if (!LoadButtonNameArrayLocked(buttons_json)) {
    logging::Log("Failed to load observed buttons");
  }

  const String commands_json = ReadString(file);
  if (!LoadCommandArrayLocked(commands_json)) {
    logging::Log("Failed to load commands");
  }

  file.close();

  logging::Log("Loaded the command table: %d buttons, %d commands",
               button_names_.size(), registered_commands_.size());
}

void CommandTable::Reset() {
  const kb::LockGuard lock(mutex_);
  observed_buttons_.clear();
  registered_commands_.clear();
  button_names_.clear();
  SPIFFS.remove(kCommandTablePath);
}

void CommandTable::SetCommandLocked(const KButton& button,
                                    const Command& command) {
  if (button_names_.count(button) == 0) {
    const String name = "ボタン" + String(g_settings.GetNextButtonId());
    SetButtonNameLocked(button, name);
  }
  DeleteCommandLocked(button);
  registered_commands_.push_back(ButtonCommandPair{button, command});
}

void CommandTable::DeleteCommandLocked(const KButton& button) {
  registered_commands_.erase(
      std::remove_if(registered_commands_.begin(), registered_commands_.end(),
                     [&button](const ButtonCommandPair& pair) {
                       return pair.button == button;
                     }),
      registered_commands_.end());
}

void CommandTable::SetButtonNameLocked(const KButton& button,
                                       const String& name) {
  button_names_[button] = name;
}
