#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize WiFi in station mode
 * 
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return ESP_OK if successful
 */
esp_err_t wifi_init_sta(const char *ssid, const char *password);

/**
 * @brief Initialize SNTP for time synchronization
 * 
 * @return ESP_OK if successful
 */
esp_err_t sntp_init_sync(void);

#ifdef __cplusplus
}
#endif
