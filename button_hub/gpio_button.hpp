#pragma once

#include <functional>

#include "command_table.hpp"

namespace gpio_button {

void Register(CommandTable& command_table);
void Unregister(CommandTable& command_table);
void HandleEvents(const std::function<void(const KButton&)>& callback);

}  // namespace gpio_button
