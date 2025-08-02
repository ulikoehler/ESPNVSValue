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

// Call this in setup() or in app_main()
void InitializeNVSConfig();

extern std::optional<nvs_handle_t> nvsHandle;
```

NVSConfig.cpp
```c++
#include "NVSConfig.hpp"
#include "NVSUtils.hpp" // InitializeNVS()

#include <Arduino.h>

std::optional<nvs_handle_t> nvsHandle = std::nullopt;

// System description for serial number
NVSStringValue serialNumber;

NVSValue<float> voltages[NumChannels];

void InitializeNVSConfig() {
    nvsHandle = InitializeNVS(/*namespace=*/"storage");
    if (!nvsHandle.has_value()) {
        ESP_LOGI("MyProductNVS", "NVS could not be initialized");
        return; // Do not continue if NVS could not be initialized
    }

    // Read or initialize values
    description = NVSStringValue(nvsHandle.value(), "description", /*default=*/"");

    voltages[0] = NVSValue<float>(nvsHandle.value(), "channel1Voltage", /*default=*/0.1f);
    voltages[1] = NVSValue<float>(nvsHandle.value(), "channel2Voltage", /*default=*/0.1f);
    voltages[2] = NVSValue<float>(nvsHandle.value(), "channel3Voltage", /*default=*/0.1f);
}
```
