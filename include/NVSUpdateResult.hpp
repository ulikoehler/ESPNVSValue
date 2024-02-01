#pragma once
#include <cstdint>

enum class NVSUpdateResult : int8_t {
    Updated = 0,
    Unchanged = 1,
    NotInitialized = -1,
    Nullptr = -2,
    Error = -3
};

const char* NVSUpdateResultToString(NVSUpdateResult updateResult);