#include "fetch_state.hpp"

#include "api.hpp"
#include "api_mutex.hpp"
#include "common.hpp"
#include "mutex.hpp"
#include "server.hpp"
#include "to_json.hpp"
#include "types.hpp"

namespace fetch_state {

static kb::Mutex g_mutex;
enum class State {
  kUninitialized,
  kFetching,
  kCompleted,
};
static State g_state = State::kUninitialized;

// for throttle
static int32_t g_last_fetch = 0;
static constexpr int32_t kInterval = 30 * 1000;

static void FetchImpl(RobotInfoHolder& out) {
  server::SendToWs(to_json::ConvertRobotInfo(out));

  while (!out.has_robot_version) {
    auto [code, robot_version] = api::GetRobotVersion();
    if (code == api::ResultCode::kOk) {
      out.robot_version = std::move(robot_version);
      Serial.printf(" * robot_version = %s\n", out.robot_version.c_str());
      out.has_robot_version = true;
      server::SendToWs(to_json::ConvertRobotInfo(out));
    } else {
      Serial.printf("Failed to get version: %s\n",
                    api::ResultCodeToString(code));
      delay(3000);
    }
  }
  delay(100);
  bool done = false;
  while (!done) {
    auto [code, shelves] = api::GetShelves();
    if (code == api::ResultCode::kOk) {
      out.shelves = std::move(shelves);
      for (const auto& [id, name] : out.shelves) {
        Serial.printf(" * %s: %s\n", id.c_str(), name.c_str());
      }
      done = true;
      out.has_shelves = true;
      server::SendToWs(to_json::ConvertRobotInfo(out));
    } else {
      Serial.printf("Failed to get shelves: %s\n",
                    api::ResultCodeToString(code));
      delay(3000);
    }
  }
  delay(100);
  done = false;
  while (!done) {
    auto [code, locations] = api::GetLocations();
    if (code == api::ResultCode::kOk) {
      out.locations = std::move(locations);
      for (const auto& [id, name] : out.locations) {
        Serial.printf(" * %s: %s\n", id.c_str(), name.c_str());
      }
      done = true;
      out.has_locations = true;
      server::SendToWs(to_json::ConvertRobotInfo(out));
    } else {
      Serial.printf("Failed to get locations: %s\n",
                    api::ResultCodeToString(code));
      delay(3000);
    }
  }
  {
    const kb::LockGuard lock(g_mutex);
    g_state = State::kCompleted;
  }
}

void RunFetchTask(void* arg) {
  RobotInfoHolder& robot_info = *static_cast<RobotInfoHolder*>(arg);
  if (const kb::LockGuard lock(api_mutex); lock) {
    FetchImpl(robot_info);
  }
  vTaskDelete(nullptr);
}

bool IsCompleted() {
  const kb::LockGuard lock(g_mutex);
  return g_state == State::kCompleted;
}

void FetchRobotInfo(RobotInfoHolder* robot_info) {
  {
    const kb::LockGuard lock(g_mutex);
    switch (g_state) {
      case State::kUninitialized:
      case State::kCompleted:
        g_state = State::kFetching;
        break;
      case State::kFetching:
        Serial.println("FetchRobotInfo: already running");
        return;
    }
  }
  xTaskCreate(RunFetchTask, "Fetch", 2 * 1024, robot_info, 2, nullptr);
}

void FetchRobotInfoThrottled(RobotInfoHolder* robot_info) {
  const auto now = static_cast<int32_t>(millis());
  if (IsCompleted() && (now - g_last_fetch) > kInterval) {
    g_last_fetch = now;
    FetchRobotInfo(robot_info);
  }
}

}  // namespace fetch_state
