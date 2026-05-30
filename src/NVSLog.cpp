#define ESPNVSVALUE_LOG_KEEP_SYMBOLS
#include "NVSLog.hpp"
#include <cstdio>
#include <cstdarg>

volatile NVSLogLevel _logLevel = NVSLogLevel::Info;

void NVSSetLogLevel(NVSLogLevel level) {
    _logLevel = level;
}

void NVSVPrintf(NVSLogLevel level, const char* format, va_list args) {
    if(level <= _logLevel) {
        printf("[NVS] [%s] ", NVSLogLevelToString(level));
        vprintf(format, args);
        printf("\n");
    }
}

// For NVS high level driver
__attribute__ ((weak)) void NVSPrintf(NVSLogLevel level, const char * format, ... ) {
    va_list args;
    va_start(args, format);
    NVSVPrintf(level, format, args);
    va_end(args);
}

__attribute__ ((weak)) void NVSCriticalPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    NVSVPrintf(NVSLogLevel::Critical, format, args);
    va_end(args);
}

__attribute__ ((weak)) void NVSErrorPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    NVSVPrintf(NVSLogLevel::Error, format, args);
    va_end(args);
}

__attribute__ ((weak)) void NVSWarningPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    NVSVPrintf(NVSLogLevel::Warning, format, args);
    va_end(args);
}

__attribute__ ((weak)) void NVSInfoPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    NVSVPrintf(NVSLogLevel::Info, format, args);
    va_end(args);
}

__attribute__ ((weak)) void NVSDebugPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    NVSVPrintf(NVSLogLevel::Debug, format, args);
    va_end(args);
}

__attribute__ ((weak)) void NVSTracePrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    NVSVPrintf(NVSLogLevel::Trace, format, args);
    va_end(args);
}

const char* NVSLogLevelToString(NVSLogLevel level) {
    switch(level) {
        case NVSLogLevel::Critical:
            return "Critical";
        case NVSLogLevel::Error:
            return "Error";
        case NVSLogLevel::Warning:
            return "Warning";
        case NVSLogLevel::Info:
            return "Info";
        case NVSLogLevel::Debug:
            return "Debug";
        case NVSLogLevel::Trace:
            return "Trace";
        default:
            return "Unknown";
    }
}