#include "ota.hpp"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <M5Unified.h>
#include <Update.h>
#include <tuple>

#include "logging.hpp"
#include "settings.hpp"
#include "version.hpp"

namespace ota {

static std::function<void()> g_on_start = []() {
};
static std::function<void(double)> g_on_progress = [](double) {
};
static std::function<void(bool)> g_on_end = [](bool) {
};
static std::function<void(Error)> g_on_error = [](Error) {
};

static String g_url;
static int64_t g_last_update = 0;
static constexpr int64_t kMaxFetchTrial = 4;
static constexpr int64_t kFetchRetryIntervalMsec = 5000;
static constexpr int64_t kWatchDogWarningMsec = 60 * 1000;
static constexpr int64_t kWatchDogRebootMsec = 60 * 1000 + kWatchDogWarningMsec;

void Begin(const std::function<void()> on_start,
           std::function<void(double /* percent */)> on_progress,
           std::function<void(bool /* success */)> on_end,
           std::function<void(Error)> on_error) {
  g_on_start = std::move(on_start);
  g_on_progress = std::move(on_progress);
  g_on_end = std::move(on_end);
  g_on_error = std::move(on_error);
}

static bool FetchAndOta(const String& url) {
  Error error = Error::kNoError;
  for (int i = 0; i < kMaxFetchTrial; ++i) {
    HTTPClient http;
    http.begin(url);
    const int http_code = http.GET();
    if (http_code != HTTP_CODE_OK) {
      logging::Log("OTA: HTTP GET failed, code=%d, error: %s", http_code,
                   HTTPClient::errorToString(http_code).c_str());
      error = Error::kHttpGetFailed;
      delay(1000);
      continue;
    }
    const size_t content_length = http.getSize();
    if (!Update.begin(content_length)) {
      logging::Log("OTA: Failed to begin OTA update: %s", Update.errorString());
      // NO_ERROR corresponds to malloc failure. See Updater.cpp
      static const String KMallocFailed = "No Error";
      error = KMallocFailed == Update.errorString() ? Error::kNotEnoughMemory
                                                    : Error::kNotEnoughSpace;
      Update.abort();
      delay(1000);
      continue;
    }

    size_t written = 0;
    {
      WiFiClient& client = http.getStream();
      uint8_t buf[2048];
      while (written < content_length) {
        const int len = client.read(buf, sizeof(buf));
        if (len > 0) {
          Update.write(buf, len);
          written += len;

          const int64_t now = static_cast<int64_t>(millis());
          if (now > g_last_update + 1000 || written == content_length) {
            const double percent = (100.0 * written) / content_length;
            logging::Log("OTA: Progress %5.1f%%", percent);
            g_on_progress(percent);
            g_last_update = now;
          }
        } else {
          delay(10);
        }
      }
    }
    if (written != content_length) {
      logging::Log("OTA: Written only partial file");
      error = Error::kWrittenOnlyPartialFile;
      Update.abort();
      delay(1000);
      continue;
    }
    // success
    error = Error::kNoError;
    break;
  }
  if (error != Error::kNoError) {
    g_on_error(error);
    return false;
  }
  if (!Update.end()) {
    logging::Log("OTA: Update error: %s", Update.errorString());
    g_on_error(Error::kVerificationFailed);
    return false;
  }
  logging::Log("OTA: Update successfully completed. Rebooting...");
  return true;
}

static void RunOtaWatchDog(void*) {
  static Error last_error = Error::kNoError;
  while (!g_url.isEmpty()) {
    const int64_t now = static_cast<int64_t>(millis());
    const int64_t elapsed = now - g_last_update;
    if (elapsed > kWatchDogWarningMsec) {
      const int64_t deadline = g_last_update + kWatchDogRebootMsec;
      const int64_t diff = deadline - now;
      Error error = Error::kWatchdogNow;
      if (diff > 60 * 1000) {
        error = Error::kWatchdog180Sec;
      } else if (diff > 30 * 1000) {
        error = Error::kWatchdog60Sec;
      } else if (diff > 10 * 1000) {
        error = Error::kWatchdog30Sec;
      } else if (diff > 0) {
        error = Error::kWatchdog10Sec;
      } else {
        error = Error::kWatchdogNow;
      }
      if (last_error != error) {
        if (error == Error::kWatchdogNow) {
          logging::Log("OTA: watchdog: now");
        } else {
          logging::Log("OTA: watchdog: %d sec",
                       error == Error::kWatchdog10Sec    ? 10
                       : error == Error::kWatchdog30Sec  ? 30
                       : error == Error::kWatchdog60Sec  ? 60
                       : error == Error::kWatchdog180Sec ? 180
                                                         : -1);
        }
        g_on_error(error);  // should be reboot in the callback if needed
        last_error = error;
      }
    } else {
      if (last_error != Error::kNoError) {
        g_on_error(Error::kNoError);
        last_error = Error::kNoError;
      }
    }
    delay(500);
  }
  vTaskDelete(nullptr);
}

static void RunOtaTask(void*) {
  g_last_update = static_cast<int64_t>(millis());
  BaseType_t retv = xTaskCreate(RunOtaWatchDog, "OtaWatchDog", 2 * 1024,
                                nullptr, 10, nullptr);
  if (retv != pdPASS) {
    logging::Log("OTA: Failed to start OTA watchdog task");
  }
  g_on_start();
  const bool ok = FetchAndOta(g_url);
  g_on_end(ok);
  g_url = "";
  vTaskDelete(nullptr);
}

bool StartOtaByUrl(const String& ota_url) {
  if (!g_url.isEmpty()) {
    logging::Log("OTA: OTA is already in progress");
    return false;
  }
  g_url = ota_url;
  BaseType_t retv =
      xTaskCreate(RunOtaTask, "Ota", 8 * 1024, nullptr, 3, nullptr);
  if (retv != pdPASS) {
    logging::Log("OTA: Failed to start OTA task");
    g_url = "";
    return false;
  }
  return true;
}

static std::tuple<bool /* success */, int /* major */, int /* minor */,
                  int /* patch */>
ParseVersion(const String& version) {
  const int dot1 = version.indexOf('.');
  if (dot1 < 0) {
    return {false, 0, 0, 0};
  }
  const int dot2 = version.indexOf('.', dot1 + 1);
  if (dot2 < 0) {
    return {false, 0, 0, 0};
  }
  // Check if each string piece is really a number
  for (int i = 0; i < version.length(); ++i) {
    if (i == dot1 || i == dot2) {
      continue;
    }
    if (!isdigit(version[i])) {
      return {false, 0, 0, 0};
    }
  }
  if (dot1 == 0 || dot2 == 0 || dot1 + 1 == dot2 ||
      dot2 + 1 == version.length()) {
    return {false, 0, 0, 0};
  }
  const int major = version.substring(0, dot1).toInt();
  const int minor = version.substring(dot1 + 1, dot2).toInt();
  const int patch = version.substring(dot2 + 1).toInt();
  return {true, major, minor, patch};
}

bool CheckOtaIsRequired(const String& current_version,
                        const String& new_version) {
  const auto [success1, major1, minor1, patch1] = ParseVersion(current_version);
  if (!success1) {
    // Unformed version means "development version".
    return false;
  }
  const auto [success2, major2, minor2, patch2] = ParseVersion(new_version);
  if (!success2) {
    return false;
  }
  if (major1 != major2) {
    return major1 < major2;
  }
  if (minor1 != minor2) {
    return minor1 < minor2;
  }
  return patch1 < patch2;
}

String GetDesiredHubVersion(const String& ota_endpoint) {
  if (ota_endpoint.isEmpty()) {
    return "";
  }

  // Cache the result to avoid unnecessary network access
  static String s_version;
  if (!s_version.isEmpty()) {
    return s_version;
  }

  String url = ota_endpoint + "/desired-version?button_hub_version=" + kVersion;
  Serial.printf("URL: \"%s\"\n", url.c_str());
  for (int i = 0; i < kMaxFetchTrial; i++) {
    logging::Log("OTA: Fetching OTA info, trial=%d/%d", i + 1, kMaxFetchTrial);
    HTTPClient http;
    http.begin(url);
    const int http_code = http.GET();
    if (http_code < 200 || 299 < http_code) {
      logging::Log("OTA: Obtaining OTA info failed, code=%d, error: %s",
                   http_code, HTTPClient::errorToString(http_code).c_str());
      delay(kFetchRetryIntervalMsec);
      continue;
    }
    String body = http.getString();

    JsonDocument doc;
    if (deserializeJson(doc, body)) {
      logging::Log("OTA: Failed to parse OTA info");
      delay(kFetchRetryIntervalMsec);
      continue;
    }
    s_version = doc["desired_button_hub_version"].as<String>();

    return s_version;
  }
  return "";
}

String GetOtaImageUrlByVersion(const String& ota_endpoint,
                               const String& version) {
  for (int i = 0; i < kMaxFetchTrial; i++) {
    String data = "{\"button_hub_version\": \"" + version + "\"}";

    HTTPClient http;
    http.begin(ota_endpoint + "/software-files-url");
    const int http_code = http.POST(std::move(data));
    if (http_code < 200 || 299 < http_code) {
      logging::Log("OTA: Obtaining OTA URL failed, code=%d, error: %s",
                   http_code, HTTPClient::errorToString(http_code).c_str());
      delay(1000);
      continue;
    }
    String body = http.getString();

    JsonDocument doc;
    if (deserializeJson(doc, body)) {
      logging::Log("OTA: Failed to parse OTA info");
      delay(1000);
      continue;
    }
    const String& url = doc["download_url"].as<String>();
    Serial.printf("URL: %s\n", url.c_str());
    return url;
  }
  return "";
}

void RebootForOtaAfterBoot(const String& url) {
  g_settings.SetOtaUrlAfterBoot(url);
  delay(1000);
  ESP.restart();
}

String GetOtaImageUrlFromServerIfAvailable(const String& ota_endpoint) {
  logging::Log("OTA: Starting automatic OTA check...");
  const String version = GetDesiredHubVersion(ota_endpoint);
  if (version.isEmpty()) {
    logging::Log("OTA: Failed to obtain desired version");
    return "";
  }
  if (!CheckOtaIsRequired(kVersion, version)) {
    logging::Log("OTA: OTA is not required: current=\"%s\", new=\"%s\"",
                 kVersion, version.c_str());
    return "";
  }
  logging::Log("OTA: Desired version: %s", version.c_str());
  String url = GetOtaImageUrlByVersion(ota_endpoint, version);
  if (url.isEmpty()) {
    logging::Log("OTA: Failed to obtain URL");
    return "";
  }
  return url;
}

}  // namespace ota
