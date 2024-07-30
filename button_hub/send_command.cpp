#include "send_command.hpp"

#include <HTTPClient.h>
#include <M5Unified.h>

#include "api.hpp"
#include "logging.hpp"
#include "robot_version.hpp"
#include "types.hpp"

namespace send_command {
namespace {
constexpr double kValidLockDurationSecThreshold = 0.001;
}

static String ResolveShelfName(const RobotInfoHolder& robot_info,
                               const String& shelf_id) {
  for (const auto& [id, name] : robot_info.shelves) {
    if (id == shelf_id) {
      return name;
    }
  }
  if (shelf_id.isEmpty() || shelf_id == "__THIS__") {
    return "載せている家具";
  }
  return shelf_id;
}

static const String& ResolveLocationName(const RobotInfoHolder& robot_info,
                                         const String& location_id) {
  for (const Location& location : robot_info.locations) {
    if (location.id == location_id) {
      return location.name;
    }
  }
  return location_id;
}

static const String& ResolveShortcutName(const RobotInfoHolder& robot_info,
                                         const String& shortcut_id) {
  for (const auto& [id, name] : robot_info.shortcuts) {
    if (id == shortcut_id) {
      return name;
    }
  }
  return shortcut_id;
}

static String GenerateTitle(const RobotInfoHolder& robot_info,
                            const Command& command) {
  switch (command.type) {
    case CommandType::MOVE_SHELF:
      return ResolveShelfName(robot_info, command.move_shelf.target_shelf_id) +
             "を" +
             ResolveLocationName(robot_info,
                                 command.move_shelf.destination_location_id) +
             "に移動";
    case CommandType::RETURN_SHELF:
      return ResolveShelfName(robot_info,
                              command.return_shelf.target_shelf_id) +
             "を片付ける";
    case CommandType::UNDOCK_SHELF:
      return "持っている家具をその場に置く";
    case CommandType::MOVE_TO_LOCATION:
      return ResolveLocationName(robot_info,
                                 command.move_to_location.target_location_id) +
             "に移動";
    case CommandType::SHORTCUT:
      return ResolveShortcutName(robot_info,
                                 command.shortcut.target_shortcut_id) +
             "を実行";
    case CommandType::RETURN_HOME:
      return "充電ドックに戻る";
    case CommandType::SPEAK:
      return "「" + command.speak.text + "」の発話";
  }
  return "";
}

enum class HttpMethod { GET, POST };

static bool FetchHttp(const HttpMethod method, const String& url,
                      const String& body) {
  HTTPClient http;
  http.begin(url);
  const int http_code =
      method == HttpMethod::GET
          ? http.GET()
          : http.POST(
                reinterpret_cast<uint8_t*>(const_cast<char*>(body.c_str())),
                body.length());
  if (http_code < 200 || 299 < http_code) {
    logging::Log("HTTP %s failed, code=%d, error: %s",
                 method == HttpMethod::GET ? "GET" : "POST", http_code,
                 HTTPClient::errorToString(http_code).c_str());
    return false;
  }
  logging::Log("HTTP response: %s", http.getString().c_str());
  return true;
}

bool SendCommand(const RobotInfoHolder& robot_info, const Command& command) {
  LockOnEnd lock_on_end;
  if (command.lock_duration_sec > kValidLockDurationSecThreshold) {
    lock_on_end.enabled = true;
    lock_on_end.duration_sec = command.lock_duration_sec;
  }

  api::ResultCode result = api::ResultCode::kOk;
  switch (command.type) {
    case CommandType::MOVE_SHELF:
      result =
          api::MoveShelf(command.move_shelf.target_shelf_id.c_str(),
                         command.move_shelf.destination_location_id.c_str(),
                         command.cancel_all, command.tts_on_success.c_str(),
                         command.deferrable, lock_on_end,
                         GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::RETURN_SHELF:
      result = api::ReturnShelf(
          command.return_shelf.target_shelf_id.c_str(), command.cancel_all,
          command.tts_on_success.c_str(), command.deferrable, lock_on_end,
          GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::UNDOCK_SHELF:
      result =
          api::UndockShelf(command.cancel_all, command.tts_on_success.c_str(),
                           command.deferrable, lock_on_end,
                           GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::MOVE_TO_LOCATION:
      result = api::MoveToLocation(
          command.move_to_location.target_location_id.c_str(),
          command.cancel_all, command.tts_on_success.c_str(),
          command.deferrable, lock_on_end,
          GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::RETURN_HOME:
      result =
          api::ReturnHome(command.cancel_all, command.tts_on_success.c_str(),
                          command.deferrable, lock_on_end,
                          GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::SHORTCUT:
      result = api::StartShortcut(
          command.shortcut.target_shortcut_id.c_str(), command.cancel_all,
          command.tts_on_success.c_str(), command.deferrable, lock_on_end,
          GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::SPEAK:
      result =
          api::Speak(command.speak.text.c_str(), command.cancel_all,
                     command.tts_on_success.c_str(), command.deferrable,
                     lock_on_end, GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::PROCEED:
      result = api::Proceed();
      break;
    case CommandType::CANCEL_COMMAND:
      result = api::CancelCommand();
      break;
    case CommandType::HTTP_GET:
      FetchHttp(HttpMethod::GET, command.http_get.url, "");
      break;
    case CommandType::HTTP_POST:
      FetchHttp(HttpMethod::POST, command.http_post.url,
                command.http_post.body);
      break;
    default:
      Serial.printf("Unknown command: %d\n", command.type);
      return false;
  }
  if (result != api::ResultCode::kOk) {
    Serial.printf("Failed to send command: %s\n",
                  api::ResultCodeToString(result));
    return false;
  }
  // start lock command for backward compatibility
  // (lock_on_end option is supported from v3.1.0)
  const bool is_lock_on_end_option_supported =
      robot_info.has_robot_version and
      RobotVersion(robot_info.robot_version) >= RobotVersion(3, 1, 0);
  if (command.lock_duration_sec > kValidLockDurationSecThreshold and
      not is_lock_on_end_option_supported) {
    Serial.printf("Lock: %d sec\n", command.lock_duration_sec);
    delay(3000);
    api::Lock(command.lock_duration_sec,
              (String(int(command.lock_duration_sec)) + "秒の待機").c_str());
  }
  return true;
}

}  // namespace send_command
