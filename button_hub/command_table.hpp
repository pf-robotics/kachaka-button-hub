#pragma once

#include <Arduino.h>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <deque>
#include <map>

#include "mutex.hpp"

struct AppleIBeacon {
  uint8_t address[6];
  uint8_t uuid[16];
  uint16_t major;
  uint16_t minor;

  explicit AppleIBeacon() = default;
  explicit AppleIBeacon(const uint8_t address[6], const uint8_t uuid[16],
                        uint16_t major, uint16_t minor)
      : major(major), minor(minor) {
    std::memcpy(this->address, address, 6);
    std::memcpy(this->uuid, uuid, 16);
  }
};

struct M5Button {
  uint8_t id;  // 1,2,3

  explicit M5Button() = default;
  explicit M5Button(const int new_id) : id(new_id) {}
};

enum class ButtonType : uint8_t {
  kAppleIBeacon,
  kM5Button,
};

struct KButton {
  ButtonType type;
  union {
    AppleIBeacon apple_i_beacon;
    M5Button m5_button;
  } data;

  explicit KButton() = default;
  explicit KButton(const AppleIBeacon& beacon)
      : type(ButtonType::kAppleIBeacon) {
    data.apple_i_beacon = beacon;
  }
  explicit KButton(const M5Button& m5_button) : type(ButtonType::kM5Button) {
    data.m5_button.id = m5_button.id;
  }
};

bool SerializeAddressToString(const uint8_t address[6], char* out, int len);
String SerializeAddressToString(const uint8_t address[6]);
bool DeserializeAddressToString(const char* data, int len, uint8_t out[6]);

bool SerializeUuidToString(const uint8_t uuid[16], char* out, int len);
String SerializeUuidToString(const uint8_t uuid[16]);
bool DeserializeUuidToString(const char* data, int len, uint8_t out[16]);

struct ObservedButton {
  std::time_t timestamp;
  double estimated_distance;  // -1 (unavailable) or positive value in meters
  KButton button;
};

inline bool operator<(const AppleIBeacon& lhs, const AppleIBeacon& rhs) {
  int cmp = std::memcmp(lhs.address, rhs.address, sizeof(lhs.address));
  if (cmp != 0) {
    return cmp < 0;
  }
  cmp = std::memcmp(lhs.uuid, rhs.uuid, sizeof(lhs.uuid));
  if (cmp != 0) {
    return cmp < 0;
  }
  cmp = lhs.major - rhs.major;
  if (cmp != 0) {
    return cmp < 0;
  }
  return lhs.minor < rhs.minor;
}

inline bool operator<(const M5Button& lhs, const M5Button& rhs) {
  return lhs.id < rhs.id;
}

inline bool operator<(const KButton& lhs, const KButton& rhs) {
  if (lhs.type == ButtonType::kAppleIBeacon &&
      rhs.type == ButtonType::kAppleIBeacon) {
    return lhs.data.apple_i_beacon < rhs.data.apple_i_beacon;
  }
  if (lhs.type == ButtonType::kM5Button && rhs.type == ButtonType::kM5Button) {
    return lhs.data.m5_button < rhs.data.m5_button;
  }
  return lhs.type < rhs.type;
}

inline bool operator==(const AppleIBeacon& lhs, const AppleIBeacon& rhs) {
  return std::memcmp(lhs.address, rhs.address, sizeof(lhs.address)) == 0 &&
         std::memcmp(lhs.uuid, rhs.uuid, sizeof(lhs.uuid)) == 0 &&
         lhs.major == rhs.major && lhs.minor == rhs.minor;
}

inline bool operator==(const M5Button& lhs, const M5Button& rhs) {
  return lhs.id == rhs.id;
}

inline bool operator==(const KButton& lhs, const KButton& rhs) {
  if (lhs.type == ButtonType::kAppleIBeacon &&
      rhs.type == ButtonType::kAppleIBeacon) {
    return lhs.data.apple_i_beacon == rhs.data.apple_i_beacon;
  }
  if (lhs.type == ButtonType::kM5Button && rhs.type == ButtonType::kM5Button) {
    return lhs.data.m5_button.id == rhs.data.m5_button.id;
  }
  return lhs.type == rhs.type;
}

inline bool operator<(const ObservedButton& lhs, const ObservedButton& rhs) {
  return lhs.timestamp < rhs.timestamp && lhs.button < rhs.button;
}

enum class CommandType : uint16_t {
  MOVE_SHELF = 1,
  RETURN_SHELF = 2,
  FOLLOW_PERSON = 3,
  MAKE_NEW_MAP = 4,
  UNDOCK_SHELF = 5,
  MOVE_TO_LOCATION = 7,
  RETURN_HOME = 8,
  DOCK_SHELF = 9,
  EXEC_TASK = 10,
  SET_UP_WIFI = 11,
  SPEAK = 12,
  MOVE_TO_POSE = 13,
  REGISTER_SHELF = 14,
  LOCK = 15,
  PROCEED = 1000,
  CANCEL_COMMAND = 1001,
};

struct Command {
  CommandType type;

  struct {
    String target_shelf_id;
    String destination_location_id;
    bool undock_on_destination;
  } move_shelf;
  struct {
    String target_shelf_id;
  } return_shelf;
  struct {
    String target_shelf_id;
  } undock_shelf;
  struct {
    String target_location_id;
  } move_to_location;
  struct {
    String task_name;
    String task_arg;
  } exec_task;
  struct {
    String text;
  } speak;
  struct {
    double x;
    double y;
    double yaw;
  } move_to_pose;

  bool cancel_all;
  String tts_on_success;
  bool deferrable;

  double lock_duration_sec;
};

class CommandTable {
 public:
  CommandTable(int max_observed_buttons);

  CommandTable(const CommandTable&) = delete;
  CommandTable& operator=(const CommandTable&) = delete;

  void NotifyObservedButton(const KButton& button, double estimated_distance);
  const std::deque<ObservedButton>& GetObservedButtons() const;

  void SetCommand(const KButton& button, const Command& command);
  void DeleteCommand(const KButton& button);
  void DeleteAllCommands();
  std::map<KButton, Command> GetCommands() const;
  bool GetCommandByButton(const KButton& button, Command* command) const;

  void SetButtonName(const KButton& button, const String& name);
  void DeleteButtonName(const KButton& button);
  void DeleteAllButtonNames();
  std::map<KButton, String> GetButtonNames() const;

  void Save();
  void Load();
  void Reset();

 private:
  int max_observed_buttons_;
  mutable kb::Mutex mutex_;
  std::deque<ObservedButton> observed_buttons_;
  std::map<KButton, Command> registered_commands_;
  std::map<KButton, String> button_names_;
};
