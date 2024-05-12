# ESPNVSValue
ESP32 Non-volatile storage high-level API which allows easy mapping from custom types to NVS entries.

NVS is the [ESP32 **Non-volatile storage library**](https://www.ecosia.org/search?q=esp%20nvs&addon=opensearch), but ESPNVSValue provides a *much easier API* than NVS itself.

You can use `NVSStringValue` to store a string. You can also use `NVSValue<float>` to store a float. This works with all types including custom types.

## Usage example

NVSConfig.hpp
```c++
#pragma once

#include <nvs_flash.h>
#include <nvs.h>

#include "NVSValue.hpp"
#include "NVSStringValue.hpp"

#include <optional>

/**
 * System description
 */
extern NVSStringValue description;

// Voltages for all our channels
constexpr size_t NumChannels = 3;
extern NVSValue<float> voltages[NumChannels];

// Call this in setup()
void InitializeNVSConfig();

extern std::optional<nvs_handle_t> nvsHandle;
```

NVSConfig.cpp
```c++
#include "NVSConfig.hpp"

#include <Arduino.h>

std::optional<nvs_handle_t> nvsHandle = std::nullopt;

// System description for serial number
NVSStringValue serialNumber;

NVSValue<float> voltages[NumChannels];

static std::optional<nvs_handle_t> InitializeNVS() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Open
    nvs_handle_t handle;
    ret = nvs_open("storage", NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        return std::nullopt;
    }

    return handle;
}

void InitializeNVSConfig() {
    nvsHandle = InitializeNVS();
    if (!nvsHandle.has_value()) {
        Serial.println("NVS could not be initialized");
        return; // Do not continue if NVS could not be initialized
    }
    // NOTE: last argument is the default value
    description = NVSStringValue(nvsHandle.value(), "description", "");

    voltages[0] = NVSValue<float>(nvsHandle.value(), "channel1Voltage", 0.1f);
    voltages[1] = NVSValue<float>(nvsHandle.value(), "channel2Voltage", 0.1f);
    voltages[2] = NVSValue<float>(nvsHandle.value(), "channel3Voltage", 0.1f);
}
```
