#pragma once
#include <nvs.h>
#include <string>

void NVSPrintf(const char* format, ...);

/**
 * @brief Abstraction for binary value, represented by a std::string stored in ESP NVS
 * The string can not only be a text but any binary data (including null bytes).
 * 
 * This class will only update the NVS value if the given value has actually been changed.
 */
class NVSStringValue {
public:
    /**
     * Empty default constructor.
     * You need to assign/copy this instance to a NVSStringValue
     * before actually using it.
     */
    NVSStringValue();
    
    NVSStringValue(NVSStringValue& copy);
    NVSStringValue(NVSStringValue&& copy);
    NVSStringValue& operator=(NVSStringValue& copy);
    NVSStringValue& operator=(NVSStringValue&& copy);

    /**
     * Main constructor.
     */
    NVSStringValue(nvs_handle_t nvs, const std::string& key);

    const std::string& key() const;
    const std::string& value() const;

    /**
     * @brief Equivalent to .value().c_str()
     * 
     * @return const char* 
     */
    const char* c_str() const;

    bool empty() const;

    size_t size() const;

    /**
     * @brief Return whether the value exists in NVS
     * 
     * @return true 
     * @return false 
     */
    bool exists() const;

    /**
     * @brief Read the value from the NVS storage
     * This is automatically called in the constructor,
     * so you only need to call this if the NVS value has been updated
     */
    void updateFromNVS();

    enum class SetResult {
        Updated = 0,
        Unchanged = 1,
        NotInitialized = -1,
        Nullptr = -2,
        Error = -3
    };

    static const char* SetResultToString(SetResult setResult);

    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    SetResult set(const std::string& newValue);

    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    SetResult set(const char* newValue);


    /**
     * @brief Update the value in the NVS and in the current instance
     * The update is skipped if the new value is equal to the current value.
     */
    SetResult set(const uint8_t* data, size_t size);

    nvs_handle_t nvs;
    std::string _key;
    std::string _value;
    bool _exists;
};