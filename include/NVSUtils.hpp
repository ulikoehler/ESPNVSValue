#pragma once
#include <nvs.h>
#include <string>
#include <optional>

/**
 * @brief Result codes for NVS query operations
 */
enum class NVSQueryResult {
    OK = 0,         ///< Operation completed successfully
    NotFound = 1,   ///< Requested key was not found in NVS
    Error = -1      ///< An error occurred during the operation
};

/**
 * @brief Get the size of a value stored in NVS
 * 
 * This function queries the NVS storage to determine the size of a blob value
 * associated with the given key without actually reading the data.
 * 
 * @param nvs Handle to the NVS namespace
 * @param key The key to query for size information
 * @param size Reference to store the size of the value (only valid if result is OK)
 * @return NVSQueryResult indicating the result of the operation:
 *         - OK: Size was successfully retrieved
 *         - NotFound: Key does not exist in NVS
 *         - Error: An error occurred during the query
 */
NVSQueryResult NVSValueSize(nvs_handle_t nvs, const std::string& key, size_t& size);

/**
 * @brief Initialize NVS flash and open a namespace
 * 
 * This function initializes the NVS (Non-Volatile Storage) flash partition and
 * opens the specified namespace for read/write operations. It handles common
 * initialization errors by erasing and reinitializing the NVS partition if needed.
 * 
 * @param namespace The name of the NVS namespace to open
 * @param allowReinit Whether to allow reinitialization of the NVS partition when errors occur.
 *                    If true, the partition will be erased and reinitialized on errors like
 *                    ESP_ERR_NVS_NO_FREE_PAGES, ESP_ERR_NVS_NEW_VERSION_FOUND, or ESP_ERR_NVS_INVALID_STATE.
 *                    If false, initialization will fail on these errors.
 * @return std::optional<nvs_handle_t> containing the NVS handle if successful,
 *         or std::nullopt if initialization or opening failed
 * 
 * @note The caller is responsible for closing the returned handle using nvs_close()
 * @note If allowReinit is true and the NVS partition is corrupted or incompatible, it will be erased and reinitialized
 */
std::optional<nvs_handle_t> InitializeNVS(const char* namespace, bool allowReinit = true);