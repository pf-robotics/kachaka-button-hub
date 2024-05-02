#pragma once

namespace ping_to_robot {

void Begin(const char* robot_host);
void Stop();

void Update();  // must be called periodically (typically from loop())

}  // namespace ping_to_robot
