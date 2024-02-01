#include "NVSResult.hpp"

const char* NVSUpdateResultToString(NVSSetResult updateResult) {
    switch (updateResult) {
        case NVSSetResult::Updated: return "Updated";
        case NVSSetResult::Unchanged: return "Unchanged";
        case NVSSetResult::NotInitialized: return "NotInitialized";
        case NVSSetResult::Nullptr: return "Nullptr";
        case NVSSetResult::Error: return "Error";
        default: return "Unknown";
    }
}
