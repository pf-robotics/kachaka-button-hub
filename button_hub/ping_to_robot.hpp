#pragma once

#include <cstdint>

namespace ping_to_robot {

void Begin(const char* robot_host);
void Stop();

void Update();  // must be called periodically (typically from loop())
int64_t GetDurationFromLastSuccess();

}  // namespace ping_to_robot
