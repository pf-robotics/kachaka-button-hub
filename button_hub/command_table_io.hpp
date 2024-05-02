#pragma once

#include <Arduino.h>

#include "command_table.hpp"

namespace command_table_io {

bool LoadCommand(const String& json, CommandTable& command_table);
bool LoadCommandArray(const String& json, CommandTable& command_table);
bool LoadButtonNameArray(const String& json, CommandTable& command_table);

}  // namespace command_table_io
