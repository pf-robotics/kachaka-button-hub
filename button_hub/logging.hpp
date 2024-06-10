#pragma once

namespace logging {

void Begin(int log_unique_id);

// This function only enqueues the log message. The actual logging is done by
// Update().
void Log(const char* format, ...);

// This must be called periodically to write the log messages to the file.
void Update();

}  // namespace logging
