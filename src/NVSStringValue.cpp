#include "NVSStringValue.hpp"
#include <limits>
#include <cstring>

#include "NVSUtils.hpp"
#include "NVSUpdateResult.hpp"
#include "NVSLog.hpp"

NVSStringValue::NVSStringValue() : nvs(std::numeric_limits<nvs_handle_t>::max()), _key(), _value(), _exists(false) {
    // Not actually initialized. Can't read value from NVS
}

NVSStringValue::NVSStringValue(nvs_handle_t nvs, const std::string& key) : nvs(nvs), _key(key), _value() {
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
    /**
     * Strategy:
     *  1. Determine size of value in NVS
     *  2. Allocate temporary buffer of determined size
     *  3. Read value from NVS into temporary buffer
     *  4. Create std::string from value
     *  5. Cleanup
     */
    // Step 1: Get size of key
    size_t value_size = 0;
    switch(NVSValueSize(nvs, _key, value_size)) {
        case NVSQueryResult::OK: {
            // OK
            _exists = true;
            break;
        }
        case NVSQueryResult::NotFound: {
            // Not found, no error
            _exists = false;
            _value = "";
            NVSPrintf(NVSLogLevel::Debug, "Key %s does not exist", _key.c_str());
            return;
        }
        case NVSQueryResult::Error: {
            // Error
            NVSPrintf(NVSLogLevel::Error, "Failed to get size of NVS key %s", _key.c_str());
            return;
        }
    }
    // For debugging
    NVSPrintf(NVSLogLevel::Trace, "Found that NVS key %s has value size %d", _key.c_str(), value_size);
    // Step 2: Allocate temporary buffer to read into
    char* buf = (char*)malloc(value_size);
    // Step 3: Read value into temporary buffer.
    if((err = nvs_get_blob(nvs, _key.c_str(), buf, &value_size)) != ESP_OK) {
        // "Doesn't exist" has already been handled before, so this is an actual error.
        // We assume that the value did not change between reading the size (step 1) and now.
        // In case that assumption is value, this will fail with ESP_ERR_NVS_INVALID_LENGTH.
        // This is extremely unlikely in all usage scenarios, however.
        NVSPrintf(NVSLogLevel::Warning, "Failed to read NVS key %s: %s", _key.c_str(), esp_err_to_name(err));
        free(buf);
        return;
    }
    // Step 4: Make string
    _exists = true;
    _value = std::string(buf, value_size);
    // For debugging
    NVSPrintf(NVSLogLevel::Debug, "Key %s exists in NVS and has value %s", _key.c_str(), _value.c_str());
    // Step 5: cleanup
    free(buf);
}

/**
 * @brief Update the value in the NVS and in the current instance
 */
NVSUpdateResult NVSStringValue::set(const std::string& newValue) {
    if(nvs == std::numeric_limits<nvs_handle_t>::max()) {
        return NVSUpdateResult::NotInitialized;
    }
    if(_value == newValue) {
        return NVSUpdateResult::Unchanged;
    }
    // Update local value
    this->_value = newValue;
    this->_exists = true;
    // Write to NVS. Use set_blob to use explicit size if string contains binary data
    esp_err_t err;
    if((err = nvs_set_blob(nvs, _key.c_str(), newValue.c_str(), newValue.size())) != ESP_OK) {
        NVSPrintf(NVSLogLevel::Critical, "Failed to write NVS key %s: %s", _key.c_str(), esp_err_to_name(err));
        return NVSUpdateResult::Error;
    }
    // Save to NV storage
    nvs_commit(nvs);
    return NVSUpdateResult::Updated;
}

NVSUpdateResult NVSStringValue::set(const uint8_t* data, size_t size) {
    std::string str((const char*)data, size);
    return set(str);
}

NVSUpdateResult NVSStringValue::set(const char* newValue) {
    if(nvs == std::numeric_limits<nvs_handle_t>::max()) {
        return NVSUpdateResult::NotInitialized;
    }
    if(newValue == nullptr) {
        return NVSUpdateResult::Nullptr;
    }
    if(_value == newValue) {
        // No change. Ignore
        return NVSUpdateResult::Unchanged;
    }
    // Update local value
    size_t len = strlen(newValue);
    this->_value = std::string(newValue, len);
    // Write to NVS
    esp_err_t err;
    if((err = nvs_set_blob(nvs, _key.c_str(), _value.c_str(), len)) != ESP_OK) {
        NVSPrintf(NVSLogLevel::Critical, "Failed to write NVS key %s: %s", _key.c_str(), esp_err_to_name(err));
        return NVSUpdateResult::Error;
    }
    // For debugging
    NVSPrintf(NVSLogLevel::Trace, "Sucessfully written NVS key %s to value %s of len %d with result %d", _key.c_str(), _value.c_str(), len, err);
    // Set successfully -> exists is true.
    this->_exists = true;
    // Save to NV storage
    nvs_commit(nvs);
    return NVSUpdateResult::Updated;
}
