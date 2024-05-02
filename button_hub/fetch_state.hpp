#pragma once

#include "types.hpp"

namespace fetch_state {

bool IsCompleted();

void FetchRobotInfo(RobotInfoHolder* robot_info);
void FetchRobotInfoThrottled(RobotInfoHolder* robot_info);

}  // namespace fetch_state
