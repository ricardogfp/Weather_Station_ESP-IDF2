#ifndef WEATHER_CLIENT_H
#define WEATHER_CLIENT_H

#include <stdint.h>
#include "esp_err.h"

// Maximum string lengths
#define MAX_API_KEY_LEN 40
#define MAX_LAT_LON_LEN 10

/**
 * @brief Initialize the weather client with API key and location
 * @param api_key OpenWeatherMap API key
 * @param latitude Location latitude as a string (e.g., "40.7128")
 * @param longitude Location longitude as a string (e.g., "-74.0060")
 */
void weather_client_init(const char *api_key, const char *latitude, const char *longitude);

/**
 * @brief Fetch current weather data from OpenWeatherMap API
 * @return esp_err_t ESP_OK on success, error code on failure
 */
esp_err_t weather_client_update(void);

/**
 * @brief Get the current temperature in Celsius
 * @return float Current temperature in Celsius, or NAN if not available
 */
float weather_client_get_temperature(void);

#endif /* WEATHER_CLIENT_H */
