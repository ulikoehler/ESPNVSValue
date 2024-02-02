#include "NVSUtils.hpp"
#include "NVSLog.hpp"

NVSQueryResult NVSValueSize(nvs_handle_t nvs, const std::string& key, size_t& size) {
    esp_err_t err;
    if((err = nvs_get_blob(nvs, key.c_str(), nullptr, &size)) != ESP_OK) {
        if(err == ESP_ERR_NVS_NOT_FOUND) {
            // Not found, no error
            NVSPrintf(NVSLogLevel::Debug, "Key %s does not exist", key.c_str());
            return NVSQueryResult::NotFound;
        } else {
            NVSPrintf(NVSLogLevel::Error, "Failed to get size of NVS key %s: %s", key.c_str(), esp_err_to_name(err));
            return NVSQueryResult::Error;
        }
    }
    return NVSQueryResult::OK;
}