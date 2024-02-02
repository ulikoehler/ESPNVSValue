#pragma once
#include <cstdint>

/**
 * Negative values are errors.
 * Positive values are successes. 
 */
enum class NVSSetResult : int8_t {
    /**
     * Successful operation.
     * Value has been changed
     */
    Updated = 0,
    /**
     * Value was not changed
     */
    Unchanged = 1,
    /**
     * Error: Class has not been initialized
     */
    NotInitialized = -1,
    /**
     * Error: Nullptr was passed as argument
     */
    Nullptr = -2,
    /**
     * Error: Unknown error
     */
    Error = -3
};

static const char* NVSSetResultToString(NVSSetResult setResult);
