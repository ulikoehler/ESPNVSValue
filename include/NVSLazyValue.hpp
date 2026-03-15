#pragma once

#include <nvs.h>

#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

#include "NVSLog.hpp"
#include "NVSResult.hpp"
#include "NVSUtils.hpp"
#include "NVSValue.hpp"

/**
 * @brief Lazily read a value from NVS on every access instead of caching it locally.
 *
 * This API mirrors NVSValue where practical, but value access always performs a fresh read.
 */
template<typename T>
class NVSLazyValue : public NVSValueBase {
public:
    NVSLazyValue() : nvs(std::numeric_limits<nvs_handle_t>::max()), _key(), _default() {}
    NVSLazyValue(const NVSLazyValue&) = default;
    NVSLazyValue(NVSLazyValue&&) = default;
    NVSLazyValue& operator=(const NVSLazyValue&) = default;
    NVSLazyValue& operator=(NVSLazyValue&&) = default;

    NVSLazyValue(nvs_handle_t nvsHandle, const std::string& key, const T& defaultValue = T())
        : nvs(nvsHandle), _key(key), _default(defaultValue) {}

    NVSLazyValue(nvs_handle_t nvsHandle, const char* key, const T& defaultValue = T())
        : nvs(nvsHandle), _key(key != nullptr ? key : ""), _default(defaultValue) {}

    const std::string& key() const override {
        return _key;
    }

    bool exists() const override {
        size_t valueSize = 0;
        if(QueryValueSize(valueSize) != NVSQueryResult::OK) {
            return false;
        }
        return valueSize == sizeof(T);
    }

    std::string asString() const override {
        if constexpr (std::is_same_v<T, std::string>) {
            return value();
        } else {
            return std::to_string(value());
        }
    }

    T value() const {
        T loadedValue = _default;
        if(!TryReadValue(loadedValue)) {
            return _default;
        }
        return loadedValue;
    }

    T valueRef() const {
        return value();
    }

    const char* c_str() const = delete;
    const uint8_t* data() const = delete;

    bool empty() const {
        return !exists();
    }

    size_t size() const {
        return sizeof(T);
    }

    void updateFromNVS() {
        // Intentionally empty: values are always read on demand.
    }

    NVSSetResult set(const T& newValue) {
        return set(&newValue);
    }

    NVSSetResult set(const T* newValue) {
        if(!IsInitialized()) {
            return NVSSetResult::NotInitialized;
        }
        if(newValue == nullptr) {
            return NVSSetResult::Nullptr;
        }

        if(exists() && value() == *newValue) {
            return NVSSetResult::Unchanged;
        }

        esp_err_t err = nvs_set_blob(nvs, _key.c_str(), static_cast<const void*>(newValue), sizeof(T));
        if(err != ESP_OK) {
            NVSPrintf(NVSLogLevel::Critical, "Failed to write NVS key %s: %s", SafeKey(), esp_err_to_name(err));
            return NVSSetResult::Error;
        }
        nvs_commit(nvs);
        return NVSSetResult::Updated;
    }

    NVSSetResult set(const uint8_t* dataBuffer, size_t dataSize) {
        if(dataBuffer == nullptr) {
            return NVSSetResult::Nullptr;
        }
        if(dataSize != sizeof(T)) {
            return NVSSetResult::Error;
        }

        T parsedValue{};
        memcpy(&parsedValue, dataBuffer, sizeof(T));
        return set(parsedValue);
    }

    nvs_handle_t nvs;
    std::string _key;
    T _default;

private:
    bool IsInitialized() const {
        return nvs != std::numeric_limits<nvs_handle_t>::max() && !_key.empty();
    }

    const char* SafeKey() const {
        return _key.empty() ? "<null>" : _key.c_str();
    }

    NVSQueryResult QueryValueSize(size_t& valueSize) const {
        if(!IsInitialized()) {
            NVSPrintf(NVSLogLevel::Critical, "Invalid NVS instance or key");
            return NVSQueryResult::Error;
        }
        return NVSValueSize(nvs, _key, valueSize);
    }

    bool TryReadValue(T& loadedValue) const {
        size_t valueSize = 0;
        switch(QueryValueSize(valueSize)) {
            case NVSQueryResult::OK:
                break;
            case NVSQueryResult::NotFound:
                return false;
            case NVSQueryResult::Error:
                return false;
        }

        if(valueSize != sizeof(T)) {
            NVSPrintf(
                NVSLogLevel::Warning,
                "Size of value in NVS for key %s (%d bytes) does not match expected size %d",
                SafeKey(),
                valueSize,
                sizeof(T));
            return false;
        }

        esp_err_t err = nvs_get_blob(nvs, _key.c_str(), static_cast<void*>(&loadedValue), &valueSize);
        if(err != ESP_OK) {
            NVSPrintf(NVSLogLevel::Warning, "Failed to read NVS key %s: %s", SafeKey(), esp_err_to_name(err));
            return false;
        }
        return true;
    }
};

template<>
class NVSLazyValue<std::string> : public NVSValueBase {
public:
    NVSLazyValue() : nvs(std::numeric_limits<nvs_handle_t>::max()), _key(), _default() {}
    NVSLazyValue(const NVSLazyValue&) = default;
    NVSLazyValue(NVSLazyValue&&) = default;
    NVSLazyValue& operator=(const NVSLazyValue&) = default;
    NVSLazyValue& operator=(NVSLazyValue&&) = default;

    NVSLazyValue(nvs_handle_t nvsHandle, const std::string& key, const std::string& defaultValue = std::string())
        : nvs(nvsHandle), _key(key), _default(defaultValue) {}

    NVSLazyValue(nvs_handle_t nvsHandle, const char* key, const char* defaultValue = "")
        : nvs(nvsHandle), _key(key != nullptr ? key : ""), _default(defaultValue != nullptr ? defaultValue : "") {}

    NVSLazyValue(nvs_handle_t nvsHandle, const char* key, const std::string& defaultValue)
        : nvs(nvsHandle), _key(key != nullptr ? key : ""), _default(defaultValue) {}

    const std::string& key() const override {
        return _key;
    }

    bool exists() const override {
        size_t valueSize = 0;
        return IsInitialized() && NVSValueSize(nvs, _key, valueSize) == NVSQueryResult::OK;
    }

    std::string asString() const override {
        return value();
    }

    std::string value() const {
        if(!IsInitialized()) {
            return _default;
        }

        size_t valueSize = 0;
        switch(NVSValueSize(nvs, _key, valueSize)) {
            case NVSQueryResult::OK:
                break;
            case NVSQueryResult::NotFound:
                return _default;
            case NVSQueryResult::Error:
                return _default;
        }

        if(valueSize == 0) {
            return std::string();
        }

        std::string loadedValue(valueSize, '\0');
        esp_err_t err = nvs_get_blob(nvs, _key.c_str(), &loadedValue[0], &valueSize);
        if(err != ESP_OK) {
            NVSPrintf(NVSLogLevel::Warning, "Failed to read NVS key %s: %s", SafeKey(), esp_err_to_name(err));
            return _default;
        }
        return loadedValue;
    }

    std::string valueRef() const {
        return value();
    }

    const char* c_str() const = delete;
    const uint8_t* data() const = delete;

    bool empty() const {
        return value().empty();
    }

    size_t size() const {
        size_t valueSize = 0;
        if(exists() && NVSValueSize(nvs, _key, valueSize) == NVSQueryResult::OK) {
            return valueSize;
        }
        return _default.size();
    }

    void updateFromNVS() {
        // Intentionally empty: values are always read on demand.
    }

    NVSSetResult set(const std::string& newValue) {
        if(!IsInitialized()) {
            return NVSSetResult::NotInitialized;
        }

        if(value() == newValue) {
            return NVSSetResult::Unchanged;
        }

        esp_err_t err = nvs_set_blob(nvs, _key.c_str(), newValue.data(), newValue.size());
        if(err != ESP_OK) {
            NVSPrintf(NVSLogLevel::Critical, "Failed to write NVS key %s: %s", SafeKey(), esp_err_to_name(err));
            return NVSSetResult::Error;
        }
        nvs_commit(nvs);
        return NVSSetResult::Updated;
    }

    NVSSetResult set(const char* newValue) {
        if(newValue == nullptr) {
            return NVSSetResult::Nullptr;
        }
        return set(std::string(newValue));
    }

    NVSSetResult set(const uint8_t* dataBuffer, size_t dataSize) {
        if(dataBuffer == nullptr) {
            return NVSSetResult::Nullptr;
        }
        return set(std::string(reinterpret_cast<const char*>(dataBuffer), dataSize));
    }

    nvs_handle_t nvs;
    std::string _key;
    std::string _default;

private:
    bool IsInitialized() const {
        return nvs != std::numeric_limits<nvs_handle_t>::max() && !_key.empty();
    }

    const char* SafeKey() const {
        return _key.empty() ? "<null>" : _key.c_str();
    }
};