#include "ping_to_robot.hpp"

#include <ESPping.h>

#include "ip_resolver.hpp"
#include "mutex.hpp"
#include "screen.hpp"

namespace ping_to_robot {

static kb::Mutex g_mutex;
static String g_robot_host;
static bool g_available = false;
static bool g_ping_result = false;
static float g_ping_time = 0;
static TaskHandle_t g_task_handle = nullptr;

static constexpr bool kDebug = false;
static constexpr int kPingIntervalMs = 2000;

static void CheckPing(void*) {
  while (true) {
    if (kDebug) {
      Serial.print("Ping ... ");
    }
    const String ip_or_host =
        ip_resolver::GetCachedIpAddressOrPassthrough(g_robot_host);
    if (Ping.ping(ip_or_host.c_str(), 1)) {
      if (kDebug) {
        Serial.printf("OK  (%u, %.1f, %u)\n", Ping.minTime(),
                      Ping.averageTime(), Ping.maxTime());
      }
      if (const kb::LockGuard lock(g_mutex); lock) {
        g_available = true;
        g_ping_result = true;
        g_ping_time = Ping.averageTime();
      }
    } else {
      if (kDebug) {
        Serial.println("NG");
      }
      if (const kb::LockGuard lock(g_mutex); lock) {
        g_available = true;
        g_ping_result = false;
      }
    }

    delay(kPingIntervalMs);
  }
}

void Begin(const char* robot_host) {
  g_robot_host = robot_host;
  xTaskCreate(CheckPing, "Ping", 2 * 1024, nullptr, 1, &g_task_handle);
}

void Stop() {
  Serial.println("Stop ping task");
  if (g_task_handle) {
    vTaskDelete(g_task_handle);
    g_task_handle = nullptr;
  }
}

void Update() {
  bool available = false;
  bool result;
  float time;
  if (const kb::LockGuard lock(g_mutex); lock) {
    available = g_available;
    g_available = false;
    result = g_ping_result;
    time = g_ping_time;
  }
  if (available) {
    static int ng_count = 0;
    if (result) {
      ng_count = 0;
    } else {
      ng_count++;
    }
    screen::DrawPingResult(result, time, ng_count);
  }
}

}  // namespace ping_to_robot
