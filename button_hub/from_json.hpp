#pragma once

#include <ArduinoJson.h>

#include "command_table.hpp"

namespace from_json {

bool ConvertCommandJson(JsonObject& root, KButton& out_button,
                        Command& out_command);
bool ConvertButtonJson(JsonObject json, KButton& out);

}  // namespace from_json
