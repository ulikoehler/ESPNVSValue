#pragma once
#include <cstdarg>
#include <cstdint>

#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif

#define ESPNVSVALUE_LOG_LEVEL_CRITICAL 0
#define ESPNVSVALUE_LOG_LEVEL_ERROR 1
#define ESPNVSVALUE_LOG_LEVEL_WARNING 2
#define ESPNVSVALUE_LOG_LEVEL_INFO 3
#define ESPNVSVALUE_LOG_LEVEL_DEBUG 4
#define ESPNVSVALUE_LOG_LEVEL_TRACE 5

#ifndef CONFIG_ESPNVSVALUE_LOG_COMPILED_LEVEL
#define CONFIG_ESPNVSVALUE_LOG_COMPILED_LEVEL ESPNVSVALUE_LOG_LEVEL_TRACE
#endif

enum class NVSLogLevel : uint8_t {
    Critical = ESPNVSVALUE_LOG_LEVEL_CRITICAL,
    Error = ESPNVSVALUE_LOG_LEVEL_ERROR,
    Warning = ESPNVSVALUE_LOG_LEVEL_WARNING,
    Info = ESPNVSVALUE_LOG_LEVEL_INFO,
    Debug = ESPNVSVALUE_LOG_LEVEL_DEBUG,
    Trace = ESPNVSVALUE_LOG_LEVEL_TRACE
};

const char* NVSLogLevelToString(NVSLogLevel level);

void NVSVPrintf(NVSLogLevel level, const char* format, va_list args);

/**
 * @brief 
 * This function is used by ESPNVSValue() functions to log errors and messages.
 * 
 * By default, it is weakly linked to a function relaying the log message to printf().
 * 
 * You can specify your own function
 */
void NVSPrintf(NVSLogLevel level, const char * format, ... );

void NVSCriticalPrintf(const char* format, ...);
void NVSErrorPrintf(const char* format, ...);
void NVSWarningPrintf(const char* format, ...);
void NVSInfoPrintf(const char* format, ...);
void NVSDebugPrintf(const char* format, ...);
void NVSTracePrintf(const char* format, ...);

#ifndef ESPNVSVALUE_LOG_KEEP_SYMBOLS
#if CONFIG_ESPNVSVALUE_LOG_COMPILED_LEVEL < ESPNVSVALUE_LOG_LEVEL_CRITICAL
#define NVSCriticalPrintf(...) ((void)0)
#endif

#if CONFIG_ESPNVSVALUE_LOG_COMPILED_LEVEL < ESPNVSVALUE_LOG_LEVEL_ERROR
#define NVSErrorPrintf(...) ((void)0)
#endif

#if CONFIG_ESPNVSVALUE_LOG_COMPILED_LEVEL < ESPNVSVALUE_LOG_LEVEL_WARNING
#define NVSWarningPrintf(...) ((void)0)
#endif

#if CONFIG_ESPNVSVALUE_LOG_COMPILED_LEVEL < ESPNVSVALUE_LOG_LEVEL_INFO
#define NVSInfoPrintf(...) ((void)0)
#endif

#if CONFIG_ESPNVSVALUE_LOG_COMPILED_LEVEL < ESPNVSVALUE_LOG_LEVEL_DEBUG
#define NVSDebugPrintf(...) ((void)0)
#endif

#if CONFIG_ESPNVSVALUE_LOG_COMPILED_LEVEL < ESPNVSVALUE_LOG_LEVEL_TRACE
#define NVSTracePrintf(...) ((void)0)
#endif
#endif

/**
 * Set the global log level for the ESPNVSValue library.
 * All log levels greater than the specified level will be ignored.
 */
void NVSSetLogLevel(NVSLogLevel level);