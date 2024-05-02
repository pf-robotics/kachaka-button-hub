#include "command_table.hpp"

#include <FS.h>
#include <SPIFFS.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#include "command_table_io.hpp"
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
    Serial.printf("Failed to write int32_t: %d\n", retv);
    return false;
  }
  return true;
}

static int32_t ReadInt32(File& file) {
  int32_t value = 0;
  const size_t retv =
      file.read(reinterpret_cast<uint8_t*>(&value), sizeof(value));
  if (retv != sizeof(value)) {
    Serial.printf("Failed to read int32_t: %d\n", retv);
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
    Serial.printf("Failed to write string: %d != %u\n", size, retv);
    return false;
  }
  return true;
}

static String ReadString(File& file) {
  int32_t size = ReadInt32(file);
  std::vector<char> buf(size + 1);
  const size_t retv = file.read(reinterpret_cast<uint8_t*>(buf.data()), size);
  if (retv != size) {
    Serial.printf("Failed to read int32_t: %d\n", retv);
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
  if (button_names_.count(button) == 0) {
    const String name = "ボタン" + String(g_settings.GetNextButtonId());
    SetButtonName(button, name);
  }
  {
    const kb::LockGuard lock(mutex_);
    registered_commands_[button] = command;
  }
}

void CommandTable::DeleteCommand(const KButton& button) {
  {
    const kb::LockGuard lock(mutex_);
    registered_commands_.erase(button);
  }
  if (button.type == ButtonType::kAppleIBeacon) {
    DeleteButtonName(button);
  }
}

void CommandTable::DeleteAllCommands() {
  const kb::LockGuard lock(mutex_);
  registered_commands_.clear();
}

std::map<KButton, Command> CommandTable::GetCommands() const {
  const kb::LockGuard lock(mutex_);
  return registered_commands_;
}

bool CommandTable::GetCommandByButton(const KButton& button,
                                      Command* command) const {
  const kb::LockGuard lock(mutex_);
  const auto iter = registered_commands_.find(button);
  if (iter == registered_commands_.end()) {
    return false;
  }
  *command = iter->second;
  return true;
}

void CommandTable::SetButtonName(const KButton& button, const String& name) {
  const kb::LockGuard lock(mutex_);
  button_names_[button] = name;
}

void CommandTable::DeleteButtonName(const KButton& button) {
  const kb::LockGuard lock(mutex_);
  button_names_.erase(button);
}

void CommandTable::DeleteAllButtonNames() {
  const kb::LockGuard lock(mutex_);
  button_names_.clear();
}

std::map<KButton, String> CommandTable::GetButtonNames() const {
  const kb::LockGuard lock(mutex_);
  return button_names_;
}

void CommandTable::Save() {
  Serial.println("Save command table.");
  {
    File file = SPIFFS.open(kTemporaryCommandTablePath, "w");
    if (!file) {
      Serial.println("Failed to open the command file");
      return;
    }
    if (!WriteInt32(file, kFileVersion) ||
        !WriteString(file, to_json::ConvertObservedButtons(GetObservedButtons(),
                                                           GetButtonNames())) ||
        !WriteString(file, to_json::ConvertCommands(GetCommands()))) {
      Serial.println("Failed to write the command file");
      return;
    }
    file.flush();
  }
  if (!SPIFFS.remove(kCommandTablePath)) {
    Serial.println("Failed to remove the old file");
  }
  if (!SPIFFS.rename(kTemporaryCommandTablePath, kCommandTablePath)) {
    Serial.println(
        "Failed to swap the command file and the temporary file. The command "
        "file is lost.");
  }
}

void CommandTable::Load() {
  Serial.println("Load command table.");
  File file = SPIFFS.open(kCommandTablePath);
  if (!file) {
    Serial.println("No setting file");
    return;
  }

  const int32_t version = ReadInt32(file);
  Serial.printf("File version = %d\n", version);
  if (version != kFileVersion) {
    Serial.printf("Invalid version %d\n", version);
    return;
  }

  const String buttons_json = ReadString(file);
  if (!command_table_io::LoadButtonNameArray(buttons_json, *this)) {
    Serial.println("Failed to load observed buttons");
  }

  const String commands_json = ReadString(file);
  if (!command_table_io::LoadCommandArray(commands_json, *this)) {
    Serial.println("Failed to load commands");
  }

  file.close();
}

void CommandTable::Reset() {
  const kb::LockGuard lock(mutex_);
  observed_buttons_.clear();
  registered_commands_.clear();
  button_names_.clear();
  SPIFFS.remove(kCommandTablePath);
}
