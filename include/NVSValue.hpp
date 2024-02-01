#pragma once
#include <nvs.h>
#include <string>
#include <limits>


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
     * You need to assign/copy this instance to a NVSStringValue
     * before actually using it.
     */
    NVSStringValue() : nvs(std::numeric_limits<nvs_handle_t>::max()), _key(), _value(), _exists(false) {}
    
    NVSStringValue(NVSStringValue& copy): nvs(copy.nvs), _key(copy._key), _value(copy._value), _exists(copy._exists) {
        // Read value from NVS
        if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
            this->updateFromNVS();
        }
    }

    NVSStringValue(NVSStringValue&& copy): nvs(std::move(copy.nvs)), _key(std::move(copy._key)), _value(std::move(copy._value)), _exists(std::move(copy._exists)) {
        // Read value from NVS
        if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
            this->updateFromNVS();
        }
    }
    NVSStringValue& operator=(NVSStringValue& copy) {
        nvs = copy.nvs;
        _key = copy._key;
        _value = copy._value;

        if(nvs != std::numeric_limits<nvs_handle_t>::max()) {
            this->updateFromNVS();
        }
        return *this;
    }

    NVSStringValue& operator=(NVSStringValue&& copy) {
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
    NVSStringValue(nvs_handle_t nvs, const std::string& key) : nvs(nvs), _key(key), _value() {
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

    bool empty() const { return _exists; }

    size_t size() const { return sizeof(T); }

    /**
     * @brief Read the value from the NVS storage
     * This is automatically called in the constructor,
     * so you only need to call this if the NVS value has been updated
     */
    void updateFromNVS();

    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    NVSUpdateResult set(const std::string& newValue);

    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    NVSUpdateResult set(const char* newValue);

    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    NVSUpdateResult set(const uint8_t* data, size_t size);

    nvs_handle_t nvs;
    std::string _key;
    T _value;
    bool _exists;
};