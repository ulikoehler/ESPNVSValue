#pragma once
#include <cstdint>

enum class NVSLogLevel : uint8_t {
    Critical = 0,
    Error = 1,
    Warning = 2,
    Info = 3,
    Debug = 4,
    Trace = 5
};

const char* NVSLogLevelToString(NVSLogLevel level);

/**
 * @brief 
 * This function is used by ESPNVSValue() functions to log errors and messages.
 * 
 * By default, it is weakly linked to a function relaying the log message to printf().
 * 
 * You can specify your own function
 */
void NVSPrintf(NVSLogLevel level, const char * format, ... );

/**
 * Set the global log level for the ESPNVSValue library.
 * All log levels greater than the specified level will be ignored.
 */
void NVSSetLogLevel(NVSLogLevel level);