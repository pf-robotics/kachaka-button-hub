#pragma once

#include <Arduino.h>
#include <vector>

#include "types.hpp"

namespace api {

void SetRobotHost(String host, int port);

enum class ResultCode {
  kOk,
  kEncodeFailed,
  kFailedToCreateTask,
  kNotConnected,
  kTimeout,
};

const char* ResultCodeToString(ResultCode code);

std::pair<ResultCode, String> GetRobotVersion();
std::pair<ResultCode, std::vector<Shelf>> GetShelves();
std::pair<ResultCode, std::vector<Location>> GetLocations();

ResultCode ReturnHome(bool cancel_all, const char* tts_on_success,
                      bool deferrable, const char* title);
ResultCode MoveToLocation(const char* location_id, bool cancel_all,
                          const char* tts_on_success, bool deferrable,
                          const char* title);
ResultCode MoveShelf(const char* shelf_id, const char* location_id,
                     bool cancel_all, const char* tts_on_success,
                     bool deferrable, const char* title);
ResultCode ReturnShelf(const char* shelf_id, bool cancel_all,
                       const char* tts_on_success, bool deferrable,
                       const char* title);
ResultCode Speak(const char* text, bool cancel_all, const char* tts_on_success,
                 bool deferrable, const char* title);
ResultCode Lock(double duration_sec, const char* title);

ResultCode Proceed();
ResultCode CancelCommand();

}  // namespace api
