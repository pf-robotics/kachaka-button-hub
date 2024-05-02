#pragma once

#include "mutex.hpp"

// The mutex to protect the gRPC API calls.
// Functions in send_command and fetch_state namespaces should be guarded by
// this mutex.
extern kb::Mutex api_mutex;
