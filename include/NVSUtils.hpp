#pragma once
#include <nvs.h>
#include <string>

enum class NVSQueryResult {
    OK = 0,
    NotFound = 1,
    Error = -1
};

NVSQueryResult NVSValueSize(nvs_handle_t nvs, const std::string& key, size_t& size);