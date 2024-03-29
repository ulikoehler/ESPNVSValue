#pragma once
#include <nvs.h>
#include <string>
#include <limits>

#include "NVSLog.hpp"
#include "NVSUtils.hpp"
#include "NVSResult.hpp"

/**
 * @brief Templated value stored in NVS
 * You can use this to store any type in NVS.
 * The memory for the given value is directly allocated in the NVS.
 */
template<typename T>
class NVSValue {
public:
    /**
     * Empty default constructor.
     * You need to assign/copy this instance to a NVSValue
     * before actually using it.
     */
    NVSValue() : nvs(std::numeric_limits<nvs_handle_t>::max()), _key(), _value(), _exists(false) {}
    
    NVSValue(NVSValue& copy): nvs(copy.nvs), _key(copy._key), _value(copy._value), _exists(copy._exists) {
        // Read value from NVS
        if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
            this->updateFromNVS();
        }
    }

    NVSValue(NVSValue&& copy): nvs(std::move(copy.nvs)), _key(std::move(copy._key)), _value(std::move(copy._value)), _exists(std::move(copy._exists)) {
        // Read value from NVS
        if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
            this->updateFromNVS();
        }
    }
    NVSValue& operator=(NVSValue& copy) {
        nvs = copy.nvs;
        _key = copy._key;
        _value = copy._value;

        if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
            this->updateFromNVS();
        }
        return *this;
    }

    NVSValue& operator=(NVSValue&& copy) {
        nvs = std::move(copy.nvs);
        _key = std::move(copy._key);
        _value = std::move(copy._value);
        
        if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
            this->updateFromNVS();
        }
        return *this;
    }

    /**
     * Main constructor.
     */
    NVSValue(nvs_handle_t nvs, const std::string& key, const T& defaultValue = T()) : nvs(nvs), _key(key), _value(), _default(defaultValue) {
        this->updateFromNVS();
    }

    const std::string& key() const;

    inline T value() const { return _value; }
    inline T& valueRef() const { return _value; }

    /**
     * @brief Equivalent to .value().c_str()
     * 
     * @return const char* 
     */
    const char* c_str() const = delete;
    const uint8_t* data() const { return &_value; }

    bool empty() const { return !_exists; }
    bool exists() const { return _exists; }

    size_t size() const { return sizeof(T); }

    /**
     * @brief Read the value from the NVS storage
     * This is automatically called in the constructor,
     * so you only need to call this if the NVS value has been updated
     */
    void updateFromNVS() {
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
                // Success. Just continue.
                if(value_size == sizeof(T)) {
                    // Size matches
                    _exists = true;
                } else {
                    NVSPrintf(NVSLogLevel::Warning, "Size of value in NVS for key %s (%d bytes) does not match expected size %d", _key.c_str(), value_size, sizeof(T));
                    _exists = false;
                    _value = _default;
                    return;
                }
                break;
            }
            case NVSQueryResult::NotFound: {
                // Not found, no error
                NVSPrintf(NVSLogLevel::Debug, "Key %s does not exist", _key.c_str());
                _exists = false;
                _value = _default;
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
        // Step 3: Read value into temporary buffer.
        esp_err_t err;
        if((err = nvs_get_blob(nvs, _key.c_str(), (void*)&_value, &value_size)) != ESP_OK) {
            // "Doesn't exist" has already been handled before, so this is an actual error.
            // We assume that the value did not change between reading the size (step 1) and now.
            // In case that assumption is value, this will fail with ESP_ERR_NVS_INVALID_LENGTH.
            // This is extremely unlikely in all usage scenarios, however.
            NVSPrintf(NVSLogLevel::Warning, "Failed to read NVS key %s: %s", _key.c_str(), esp_err_to_name(err));
        }
        // Step 4: Make string
        // For debugging
        NVSPrintf(NVSLogLevel::Trace, "Key %s exists in NVS", _key.c_str());
    }

    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    NVSSetResult set(const T& newValue) {
        return set(&newValue);
    }

    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    NVSSetResult set(const T* newValue) {
        if(nvs == std::numeric_limits<nvs_handle_t>::max()) {
            return NVSSetResult::NotInitialized;
        }
        if(_value == *newValue) {
            return NVSSetResult::Unchanged;
        }
        // Update local value
        this->_value = *newValue;
        this->_exists = true;
        // Write to NVS. Use set_blob to use explicit size if string contains binary data
        esp_err_t err;
        if((err = nvs_set_blob(nvs, _key.c_str(), (void*)newValue, sizeof(T))) != ESP_OK) {
            NVSPrintf(NVSLogLevel::Critical, "Failed to write NVS key %s: %s", _key.c_str(), esp_err_to_name(err));
            return NVSSetResult::Error;
        }
        // Save to NV storage
        nvs_commit(nvs);
        return NVSSetResult::Updated;

    }

    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    NVSSetResult set(const uint8_t* data, size_t size);

    nvs_handle_t nvs;
    std::string _key;
    T _value;
    T _default;
    bool _exists;
};