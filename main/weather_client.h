#ifndef WEATHER_CLIENT_H
#define WEATHER_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "esp_err.h"

// Maximum string lengths
#define MAX_API_KEY_LEN 40
#define MAX_LAT_LON_LEN 10
#define MAX_WEATHER_DESC_LEN 32
#define MAX_WEATHER_ICON_LEN 10
#define MAX_HOURLY_FORECAST 7  // Maximum number of hourly forecasts to store
#define MAX_DAILY_FORECAST 8    // Maximum number of daily forecasts to store
#define MAX_HOURLY_FORECAST_DISPLAY 7 // Number of hourly forecasts to display on UI

// Weather condition codes mapping to icon names
typedef enum {
    WEATHER_CLEAR_SKY,
    WEATHER_FEW_CLOUDS,
    WEATHER_SCATTERED_CLOUDS,
    WEATHER_BROKEN_CLOUDS,
    WEATHER_SHOWER_RAIN,
    WEATHER_RAIN,
    WEATHER_THUNDERSTORM,
    WEATHER_SNOW,
    WEATHER_MIST,
    WEATHER_UNKNOWN
} weather_condition_t;

// Hourly forecast data structure
typedef struct {
    time_t timestamp;                // Unix timestamp of the forecast
    float temperature;               // Temperature in Celsius
    char icon[MAX_WEATHER_ICON_LEN]; // Weather icon code
    weather_condition_t condition;   // Weather condition
} hourly_forecast_t;

// Daily forecast data structure
typedef struct {
    time_t timestamp;                // Unix timestamp of the forecast day (usually noon or midnight)
    float temp_min;                  // Minimum daily temperature in Celsius
    float temp_max;                  // Maximum daily temperature in Celsius
    char icon[MAX_WEATHER_ICON_LEN]; // Weather icon code for the day
} daily_forecast_t;

/**
 * @brief Weather data structure
 */
typedef struct {
    // Current weather
    float temperature;                    // Temperature in Celsius
    char description[MAX_WEATHER_DESC_LEN]; // Weather description
    char icon[MAX_WEATHER_ICON_LEN];      // Weather icon code
    weather_condition_t condition;         // Weather condition
    
    // Hourly forecast
    hourly_forecast_t hourly[MAX_HOURLY_FORECAST]; // Hourly forecasts
    uint8_t hourly_count;                         // Number of available hourly forecasts
    time_t last_forecast_update;                  // Timestamp of last forecast update

    // Daily forecast
    daily_forecast_t daily[MAX_DAILY_FORECAST]; // Daily forecasts for 8 days
    uint8_t daily_count;                        // Number of available daily forecasts
    float temp_min;                            // Today's minimum temperature (from daily[0])
    float temp_max;                            // Today's maximum temperature (from daily[0])
} weather_data_t;

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
 * @brief Get the current weather data
 * @param[out] data Pointer to weather_data_t structure to fill
 * @return true if data is valid, false otherwise
 */
bool weather_client_get_data(weather_data_t *data);

/**
 * @brief Get the current temperature in Celsius
 * @return float Current temperature in Celsius, or NAN if not available
 */
float weather_client_get_temperature(void);

/**
 * @brief Get the current weather description
 * @param[out] buffer Buffer to store the description
 * @param size Size of the buffer
 * @return true if description was copied, false otherwise
 */
bool weather_client_get_description(char *buffer, size_t size);

/**
 * @brief Get the current weather icon code
 * @param[out] buffer Buffer to store the icon code (max 10 characters)
 * @param size Size of the buffer (should be at least MAX_WEATHER_ICON_LEN)
 * @return true if icon code was copied, false otherwise
 */
bool weather_client_get_icon(char *buffer, size_t size);

/**
 * @brief Get the current weather condition
 * @return weather_condition_t The current weather condition
 */
weather_condition_t weather_client_get_condition(void);

/**
 * @brief Get hourly forecast data for a specific hour offset (1h, 2h, etc.)
 * @param hour_offset Hours from now (1 = next hour, 2 = in 2 hours, etc.)
 * @param forecast Pointer to structure to store the forecast data
 * @return true if forecast data is available, false otherwise
 */
bool weather_client_get_hourly_forecast(uint8_t hour_offset, hourly_forecast_t *forecast);

/**
 * @brief Get the number of available hourly forecasts
 * @return Number of available hourly forecasts (0 if none)
 */
uint8_t weather_client_get_hourly_forecast_count(void);

/**
 * @brief Get daily forecast data for a specific day offset (0 = today, 1 = tomorrow, etc.)
 * @param day_offset Days from today (0 to MAX_DAILY_FORECAST - 1)
 * @param forecast Pointer to structure to store the forecast data
 * @return true if forecast data is available, false otherwise
 */
bool weather_client_get_daily_forecast(uint8_t day_offset, daily_forecast_t *forecast);

/**
 * @brief Get the number of available daily forecasts
 * @return Number of available daily forecasts (0 if none)
 */
uint8_t weather_client_get_daily_forecast_count(void);

/**
 * @brief Get the time of the next forecast update
 * @return Unix timestamp of when the next forecast update will be available
 */
time_t weather_client_get_next_forecast_update(void);

#endif /* WEATHER_CLIENT_H */
