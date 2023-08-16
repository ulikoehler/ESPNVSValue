#include "NVSUpdateResult.hpp"


const char NVSUpdateResultToString(NVSUpdateResult updateResult) {
    switch (updateResult) {
        case NVSUpdateResult::Updated: return "Updated";
        case NVSUpdateResult::Unchanged: return "Unchanged";
        case NVSUpdateResult::NotInitialized: return "NotInitialized";
        case NVSUpdateResult::Nullptr: return "Nullptr";
        case NVSUpdateResult::Error: return "Error";
        default: return "Unknown";
    }
}
