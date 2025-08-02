# ESPNVSValue
ESP32 Non-volatile storage high-level API which allows easy mapping from custom types to NVS entries.

NVS is the [ESP32 **Non-volatile storage library**](https://www.ecosia.org/search?q=esp%20nvs&addon=opensearch), but ESPNVSValue provides a *much easier API* than NVS itself.

You can use `NVSStringValue` to store a string. You can also use `NVSValue<float>` to store a float. This works with all types including custom types.

## Usage example

### `MyNVS.hpp`

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
void InitializeMyNVS();

extern std::optional<nvs_handle_t> nvsHandle;
```

### `MyNVS.cpp`

```c++
#include "MyNVS.hpp"
#include "NVSUtils.hpp" // InitializeNVS()

#include <Arduino.h>

std::optional<nvs_handle_t> nvsHandle = std::nullopt;

// System description for serial number
NVSStringValue serialNumber;

NVSValue<float> voltages[NumChannels];

void InitializeMyNVS() {
    // NOTE: Namespace has a length limit of 15 characters!
    nvsHandle = InitializeNVS(/*namespace=*/"myproduct");
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


### `main.ino` (Arduino)

For Arduino, here's how to use it:

```c++
#include "MyNVS.hpp"

void setup() {
    Serial.begin(115200);
    InitializeMyNVS();

    if (nvsHandle.has_value()) {
        Serial.println("NVS initialized successfully");
        Serial.printf("Description: %s\n", description.get().c_str());
        for (size_t i = 0; i < NumChannels; ++i) {
            Serial.printf("Channel %zu Voltage: %.2f V\n", i + 1, voltages[i].get());
        }
    } else {
        Serial.println("Failed to initialize NVS");
    }

    // Example of setting a new value
    description.set("New system description");
}
```

### `main.cpp` (ESP-IDF)

```c++
#include "MyNVS.hpp"

extern "C" void app_main() {
    InitializeMyNVS();

    if (nvsHandle.has_value()) {
        ESP_LOGI("MyProductNVS", "NVS initialized successfully");
        ESP_LOGI("MyProductNVS", "Description: %s", description.get().c_str());
        for (size_t i = 0; i < NumChannels; ++i) {
            ESP_LOGI("MyProductNVS", "Channel %zu Voltage: %.2f V", i + 1, voltages[i].get());
        }
    } else {
        ESP_LOGE("MyProductNVS", "Failed to initialize NVS");
    }

    // Example of setting a new value
    description.set("New system description");
}
```