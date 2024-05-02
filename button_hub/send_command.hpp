#pragma once

#include "command_table.hpp"
#include "types.hpp"

namespace send_command {

bool SendCommand(const RobotInfoHolder& robot_info, const Command& command);

}  // namespace send_command
