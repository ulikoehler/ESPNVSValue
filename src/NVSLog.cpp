#include "NVSLog.hpp"
#include <cstdio>

// For NVS high level driver
void NVSPrintf(NVSLogLevel level, const char * format, ... ) __attribute__ ((weak)) {
    va_list args;
    va_start(args, format);
    if(level <= NVSLogLevel::Info) {
        printf("[NVS] [%s] ", NVSLogLevelToString(level));
        vprintf(format, args);
        print("\n");
    }
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