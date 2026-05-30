#include "NVSUtils.hpp"
#include "NVSLog.hpp"

#include <nvs_flash.h>

NVSQueryResult NVSValueSize(nvs_handle_t nvs, const std::string& key, size_t& size) {
    esp_err_t err;
    if((err = nvs_get_blob(nvs, key.c_str(), nullptr, &size)) != ESP_OK) {
        if(err == ESP_ERR_NVS_NOT_FOUND) {
            // Not found, no error
            NVSDebugPrintf("Key %s does not exist", key.c_str());
            return NVSQueryResult::NotFound;
        } else {
            NVSErrorPrintf("Failed to get size of NVS key %s: %s", key.c_str(), esp_err_to_name(err));
            return NVSQueryResult::Error;
        }
    }
    return NVSQueryResult::OK;
}

namespace {
NVSQueryResult QueryBlobStringValueSize(nvs_handle_t nvs, const std::string& key, size_t& size) {
    esp_err_t err = nvs_get_blob(nvs, key.c_str(), nullptr, &size);
    if(err == ESP_OK) {
        return NVSQueryResult::OK;
    }
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        return NVSQueryResult::NotFound;
    }

    NVSErrorPrintf("Failed to get size of blob-backed NVS key %s: %s", key.c_str(), esp_err_to_name(err));
    return NVSQueryResult::Error;
}

NVSQueryResult QueryLegacyStringValueSize(nvs_handle_t nvs, const std::string& key, size_t& size) {
    esp_err_t err = nvs_get_str(nvs, key.c_str(), nullptr, &size);
    if(err == ESP_OK) {
        if(size > 0) {
            size -= 1;
        }
        return NVSQueryResult::OK;
    }
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        return NVSQueryResult::NotFound;
    }

    NVSErrorPrintf("Failed to get size of legacy string NVS key %s: %s", key.c_str(), esp_err_to_name(err));
    return NVSQueryResult::Error;
}

NVSQueryResult ReadBlobStringValue(nvs_handle_t nvs, const std::string& key, std::string& value) {
    size_t size = 0;
    esp_err_t err = nvs_get_blob(nvs, key.c_str(), nullptr, &size);
    if(err == ESP_OK) {
        if(size == 0) {
            value.clear();
            return NVSQueryResult::OK;
        }

        value.resize(size);
        if((err = nvs_get_blob(nvs, key.c_str(), value.data(), &size)) != ESP_OK) {
            NVSWarningPrintf("Failed to read blob-backed NVS key %s: %s", key.c_str(), esp_err_to_name(err));
            return NVSQueryResult::Error;
        }
        value.resize(size);
        return NVSQueryResult::OK;
    }
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        return NVSQueryResult::NotFound;
    }

    NVSWarningPrintf("Failed to query blob-backed NVS key %s: %s", key.c_str(), esp_err_to_name(err));
    return NVSQueryResult::Error;
}

NVSQueryResult ReadLegacyStringValue(nvs_handle_t nvs, const std::string& key, std::string& value) {
    size_t size = 0;
    esp_err_t err = nvs_get_str(nvs, key.c_str(), nullptr, &size);
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        return NVSQueryResult::NotFound;
    }
    if(err != ESP_OK) {
        NVSWarningPrintf("Failed to query legacy string NVS key %s: %s", key.c_str(), esp_err_to_name(err));
        return NVSQueryResult::Error;
    }

    std::string buffer(size, '\0');
    if((err = nvs_get_str(nvs, key.c_str(), buffer.data(), &size)) != ESP_OK) {
        NVSWarningPrintf("Failed to read legacy string NVS key %s: %s", key.c_str(), esp_err_to_name(err));
        return NVSQueryResult::Error;
    }

    value.assign(buffer.data(), size > 0 ? size - 1 : 0);
    return NVSQueryResult::OK;
}
} // namespace

NVSQueryResult NVSStringValueSize(nvs_handle_t nvs, const std::string& key, size_t& size, NVSStringStoragePreference preference) {
    NVSQueryResult firstResult;
    NVSQueryResult secondResult;

    if(preference == NVSStringStoragePreference::PreferString) {
        firstResult = QueryLegacyStringValueSize(nvs, key, size);
        if(firstResult != NVSQueryResult::NotFound) {
            return firstResult;
        }
        secondResult = QueryBlobStringValueSize(nvs, key, size);
    } else {
        firstResult = QueryBlobStringValueSize(nvs, key, size);
        if(firstResult != NVSQueryResult::NotFound) {
            return firstResult;
        }
        secondResult = QueryLegacyStringValueSize(nvs, key, size);
    }

    if(secondResult == NVSQueryResult::NotFound) {
        NVSDebugPrintf("Key %s does not exist", key.c_str());
    }
    return secondResult;
}

NVSQueryResult NVSReadStringValue(nvs_handle_t nvs, const std::string& key, std::string& value, NVSStringStoragePreference preference) {
    NVSQueryResult firstResult;
    NVSQueryResult secondResult;

    if(preference == NVSStringStoragePreference::PreferString) {
        firstResult = ReadLegacyStringValue(nvs, key, value);
        if(firstResult != NVSQueryResult::NotFound) {
            return firstResult;
        }
        secondResult = ReadBlobStringValue(nvs, key, value);
    } else {
        firstResult = ReadBlobStringValue(nvs, key, value);
        if(firstResult != NVSQueryResult::NotFound) {
            return firstResult;
        }
        secondResult = ReadLegacyStringValue(nvs, key, value);
    }

    if(secondResult == NVSQueryResult::NotFound) {
        NVSDebugPrintf("Key %s does not exist", key.c_str());
    }
    return secondResult;
}

std::optional<nvs_handle_t> InitializeNVS(const char* namespc, bool allowReinit) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (allowReinit && (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND || ret == ESP_ERR_NVS_INVALID_STATE)) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        nvs_flash_erase(); // Without error check
        ret = nvs_flash_init();
    }
    if(ret != ESP_OK) {
        NVSErrorPrintf("NVS flash init failed: %s", esp_err_to_name(ret));
        return std::nullopt;
    }

    // Open namespace for read/write access
    nvs_handle_t handle;
    ret = nvs_open(namespc, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        NVSErrorPrintf("Failed to open NVS namespace '%s': %s", namespc, esp_err_to_name(ret));
        return std::nullopt;
    }

    return handle;
}