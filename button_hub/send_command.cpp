#include "send_command.hpp"

#include "api.hpp"
#include "common.hpp"
#include "types.hpp"

namespace send_command {

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
  for (const auto& [id, name] : robot_info.locations) {
    if (id == location_id) {
      return name;
    }
  }
  return location_id;
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
    case CommandType::MOVE_TO_LOCATION:
      return ResolveLocationName(robot_info,
                                 command.move_to_location.target_location_id) +
             "に移動";
    case CommandType::RETURN_HOME:
      return "充電ドックに戻る";
    case CommandType::SPEAK:
      return "「" + command.speak.text + "」の発話";
  }
  return "";
}

bool SendCommand(const RobotInfoHolder& robot_info, const Command& command) {
  api::ResultCode result = api::ResultCode::kOk;
  switch (command.type) {
    case CommandType::MOVE_SHELF:
      result = api::MoveShelf(
          command.move_shelf.target_shelf_id.c_str(),
          command.move_shelf.destination_location_id.c_str(),
          command.cancel_all, command.tts_on_success.c_str(),
          command.deferrable, GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::RETURN_SHELF:
      result = api::ReturnShelf(
          command.return_shelf.target_shelf_id.c_str(), command.cancel_all,
          command.tts_on_success.c_str(), command.deferrable,
          GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::MOVE_TO_LOCATION:
      result = api::MoveToLocation(
          command.move_to_location.target_location_id.c_str(),
          command.cancel_all, command.tts_on_success.c_str(),
          command.deferrable, GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::RETURN_HOME:
      result = api::ReturnHome(
          command.cancel_all, command.tts_on_success.c_str(),
          command.deferrable, GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::SPEAK:
      result = api::Speak(command.speak.text.c_str(), command.cancel_all,
                          command.tts_on_success.c_str(), command.deferrable,
                          GenerateTitle(robot_info, command).c_str());
      break;
    case CommandType::PROCEED:
      result = api::Proceed();
      break;
    case CommandType::CANCEL_COMMAND:
      result = api::CancelCommand();
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
  if (command.lock_duration_sec > 0.001) {
    Serial.printf("Lock: %d sec\n", command.lock_duration_sec);
    vTaskDelay(100);
    api::Lock(command.lock_duration_sec,
              (String(int(command.lock_duration_sec)) + "秒の待機").c_str());
  }
  return true;
}

}  // namespace send_command
