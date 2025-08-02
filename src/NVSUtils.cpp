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

std::optional<nvs_handle_t> InitializeNVS(const char* namespace, bool allowReinit) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (allowReinit && (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND || ret == ESP_ERR_NVS_INVALID_STATE)) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        nvs_flash_erase(); // Without error check
        ret = nvs_flash_init();
    }
    if(ret != ESP_OK) {
        ESP_LOGE("ESPNVSValue", "NVS flash init failed: %s", esp_err_to_name(ret));
        return std::nullopt;
    }

    // Open namespace for read/write access
    nvs_handle_t handle;
    ret = nvs_open(namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        NVSPrintf(NVSLogLevel::Error, "Failed to open NVS namespace '%s': %s", namespace, esp_err_to_name(ret));
        return std::nullopt;
    }

    return handle;
}