#include "NVSStringValue.hpp"
#include <limits>
#include <cstring>

#include "NVSUtils.hpp"
#include "NVSResult.hpp"
#include "NVSLog.hpp"

NVSStringValue::NVSStringValue() : nvs(std::numeric_limits<nvs_handle_t>::max()), _key(), _value(), _exists(false) {
    // Not actually initialized. Can't read value from NVS
}

NVSStringValue::NVSStringValue(nvs_handle_t nvs, const std::string& key, const std::string& defaultValue) : nvs(nvs), _key(key), _value(), _default(defaultValue) {
    // Update if we didn't copy from an empty instance
    if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
        this->updateFromNVS();
    }
}

NVSStringValue::NVSStringValue(NVSStringValue& copy): nvs(copy.nvs), _key(copy._key), _value(copy._value), _exists(copy._exists) {
    // Read value from NVS
    if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
        this->updateFromNVS();
    }
}

NVSStringValue::NVSStringValue(NVSStringValue&& copy): nvs(std::move(copy.nvs)), _key(std::move(copy._key)), _value(std::move(copy._value)), _exists(std::move(copy._exists)) {
    // Read value from NVS
    if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
        this->updateFromNVS();
    }
}

NVSStringValue& NVSStringValue::operator=(NVSStringValue& copy) {
    nvs = copy.nvs;
    _key = copy._key;
    _value = copy._value;

    if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
        this->updateFromNVS();
    }
    return *this;
}

NVSStringValue& NVSStringValue::operator=(NVSStringValue&& copy) {
    nvs = std::move(copy.nvs);
    _key = std::move(copy._key);
    _value = std::move(copy._value);
    
    if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
        this->updateFromNVS();
    }
    return *this;
}

const std::string& NVSStringValue::key() const {
    return _key;
}

const std::string& NVSStringValue::value() const {
    return _value;
}

void NVSStringValue::updateFromNVS() {
    // For debugging
    NVSPrintf(NVSLogLevel::Trace, "Reading key %s", _key.c_str());
    if(nvs == std::numeric_limits<nvs_handle_t>::max()) {
        NVSPrintf(NVSLogLevel::Critical, "Invalid NVS instance");
        return;
    }
    if(NVSReadStringValue(nvs, _key, _value, NVSStringStoragePreference::PreferBlob) != NVSQueryResult::OK) {
        _exists = false;
        _value = _default;
        return;
    }

    _exists = true;
    NVSPrintf(NVSLogLevel::Debug, "Key %s exists in NVS and has %d bytes", _key.c_str(), _value.size());
}

/**
 * @brief Update the value in the NVS and in the current instance
 */
NVSSetResult NVSStringValue::set(const std::string& newValue) {
    if(nvs == std::numeric_limits<nvs_handle_t>::max()) {
        return NVSSetResult::NotInitialized;
    }
    if(_value == newValue) {
        return NVSSetResult::Unchanged;
    }
    // Update local value
    this->_value = newValue;
    this->_exists = true;
    // Write to NVS. Use set_blob to use explicit size if string contains binary data
    esp_err_t err;
    if((err = nvs_set_blob(nvs, _key.c_str(), newValue.data(), newValue.size())) != ESP_OK) {
        NVSPrintf(NVSLogLevel::Critical, "Failed to write NVS key %s: %s", _key.c_str(), esp_err_to_name(err));
        return NVSSetResult::Error;
    }
    // Save to NV storage
    nvs_commit(nvs);
    return NVSSetResult::Updated;
}

NVSSetResult NVSStringValue::set(const uint8_t* data, size_t size) {
    std::string str((const char*)data, size);
    return set(str);
}

NVSSetResult NVSStringValue::set(const char* newValue) {
    if(nvs == std::numeric_limits<nvs_handle_t>::max()) {
        return NVSSetResult::NotInitialized;
    }
    if(newValue == nullptr) {
        return NVSSetResult::Nullptr;
    }
    if(_value == newValue) {
        // No change. Ignore
        return NVSSetResult::Unchanged;
    }
    // Update local value
    size_t len = strlen(newValue);
    this->_value = std::string(newValue, len);
    // Write to NVS
    esp_err_t err;
    if((err = nvs_set_blob(nvs, _key.c_str(), _value.c_str(), len)) != ESP_OK) {
        NVSPrintf(NVSLogLevel::Critical, "Failed to write NVS key %s: %s", _key.c_str(), esp_err_to_name(err));
        return NVSSetResult::Error;
    }
    // For debugging
    NVSPrintf(NVSLogLevel::Trace, "Sucessfully written NVS key %s to value %s of len %d with result %d", _key.c_str(), _value.c_str(), len, err);
    // Set successfully -> exists is true.
    this->_exists = true;
    // Save to NV storage
    nvs_commit(nvs);
    return NVSSetResult::Updated;
}
