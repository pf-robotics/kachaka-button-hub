#pragma once

#include <Arduino.h>
#include <functional>

#include "settings.hpp"

namespace ota {

enum class Error {
  kNoError,
  kHttpGetFailed,
  kNotEnoughMemory,
  kNotEnoughSpace,
  kWrittenOnlyPartialFile,
  kVerificationFailed,
  kWatchdogNow,
  kWatchdog10Sec,
  kWatchdog30Sec,
  kWatchdog60Sec,
  kWatchdog180Sec,
};

void Begin(const std::function<void()> on_start,
           std::function<void(double /* percent */)> on_progress,
           std::function<void(bool /* success */)> on_end,
           std::function<void(Error)> on_error);

// Start OTA process on the current FreeRTOS task.
bool StartOtaByUrl(const String& ota_url);

bool CheckOtaIsRequired(const String& current_version,
                        const String& new_version);
String GetDesiredHubVersion(const String& ota_endpoint);
String GetOtaImageUrlByVersion(const String& ota_endpoint,
                               const String& version);

// Start OTA process after rebooting
void RebootForOtaAfterBoot(const String& ota_url);

String GetOtaImageUrlFromServerIfAvailable(const String& ota_endpoint);

}  // namespace ota
