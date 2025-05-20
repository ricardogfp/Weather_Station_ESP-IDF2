#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "weather_client.h"

// Logging tag
static const char *TAG = "WEATHER_CLIENT";

// API configuration
#define WEATHER_API_URL "https://api.openweathermap.org/data/2.5/onecall?exclude=minutely,daily,alerts&units=metric"

// Global variables
static char api_key[MAX_API_KEY_LEN] = {0};
static char latitude[MAX_LAT_LON_LEN] = {0};
static char longitude[MAX_LAT_LON_LEN] = {0};
static weather_data_t current_weather = {
    .temperature = NAN,
    .description = "",
    .icon = "",
    .condition = WEATHER_UNKNOWN,
    .hourly_count = 0,
    .last_forecast_update = 0
};
static char *response_buffer = NULL;
static size_t response_len = 0;

// HTTP client event handler
static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static char *local_buffer = NULL;
    static size_t total_len = 0;
    
    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If not chunked, store the data
                char* new_buffer = realloc(local_buffer, total_len + evt->data_len + 1);
                if (new_buffer == NULL) {
                    ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
                    free(local_buffer);
                    local_buffer = NULL;
                    total_len = 0;
                    return ESP_FAIL;
                }
                local_buffer = new_buffer;
                memcpy(local_buffer + total_len, evt->data, evt->data_len);
                total_len += evt->data_len;
                local_buffer[total_len] = '\0';
            }
            break;
            
        case HTTP_EVENT_ON_FINISH:
            if (local_buffer && total_len > 0) {
                // Allocate new buffer for the response
                if (response_buffer) {
                    free(response_buffer);
                }
                response_buffer = malloc(total_len + 1);
                if (response_buffer) {
                    memcpy(response_buffer, local_buffer, total_len);
                    response_buffer[total_len] = '\0';
                    response_len = total_len;
                }
            }
            free(local_buffer);
            local_buffer = NULL;
            total_len = 0;
            break;
            
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP request failed");
            free(local_buffer);
            local_buffer = NULL;
            total_len = 0;
            break;
            
        default:
            break;
    }
    return ESP_OK;
}

// Map OpenWeatherMap weather condition to our enum
static weather_condition_t map_weather_condition(const char *icon) {
    if (!icon) return WEATHER_UNKNOWN;
    
    // Check first character of icon code (e.g., '01d' -> '0')
    switch (icon[0]) {
        case '0': // 01d, 01n, etc.
            return (icon[1] == '1') ? WEATHER_CLEAR_SKY : WEATHER_UNKNOWN;
        case '2': // Thunderstorm
            return WEATHER_THUNDERSTORM;
        case '3': // Drizzle
            return WEATHER_SHOWER_RAIN;
        case '5': // Rain
            return WEATHER_RAIN;
        case '6': // Snow
            return WEATHER_SNOW;
        case '7': // Atmosphere (Mist, Smoke, Haze, etc.)
            return WEATHER_MIST;
        case '8': // Clouds
            // Check second character for cloudiness
            if (icon[1] == '0') return WEATHER_CLEAR_SKY; // 01d/n
            if (icon[1] == '1' || icon[1] == '2') return WEATHER_FEW_CLOUDS;
            if (icon[1] == '3') return WEATHER_SCATTERED_CLOUDS;
            if (icon[1] == '4') return WEATHER_BROKEN_CLOUDS;
            return WEATHER_BROKEN_CLOUDS;
        default:
            return WEATHER_UNKNOWN;
    }
}

// Parse weather data from JSON response
static esp_err_t parse_weather_data(const char *json_str) {
    if (!json_str) {
        ESP_LOGE(TAG, "NULL JSON string provided");
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_ERR_INVALID_RESPONSE;
    }

    // Get current time for forecast reference
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    // Calculate the start of the current hour
    time_t current_hour = now - (timeinfo.tm_min * 60 + timeinfo.tm_sec);
    
    // We want to start from the next full hour
    time_t forecast_start = current_hour + 3600;  // Next hour
    
    // We'll take the next 7 hours of forecast data
    time_t forecast_end = forecast_start + (7 * 3600);  // Next 7 hours from the next full hour
    
    ESP_LOGI(TAG, "Current time: %s", asctime(&timeinfo));
    ESP_LOGI(TAG, "Forecast window: %s to %s", 
             asctime(localtime(&forecast_start)), 
             asctime(localtime(&forecast_end)));

    // Initialize weather data with default values
    current_weather.temperature = NAN;
    current_weather.description[0] = '\0';
    current_weather.icon[0] = '\0';
    current_weather.condition = WEATHER_UNKNOWN;
    current_weather.hourly_count = 0;

    // Parse current weather
    cJSON *current = cJSON_GetObjectItem(root, "current");
    if (current) {
        // Get temperature
        cJSON *temp = cJSON_GetObjectItem(current, "temp");
        if (cJSON_IsNumber(temp)) {
            current_weather.temperature = temp->valuedouble;
            ESP_LOGI(TAG, "Current temperature: %.1f°C", current_weather.temperature);
        }
        
        // Get weather array (first item contains main weather info)
        cJSON *weather_array = cJSON_GetObjectItem(current, "weather");
        if (cJSON_IsArray(weather_array) && cJSON_GetArraySize(weather_array) > 0) {
            cJSON *weather = cJSON_GetArrayItem(weather_array, 0);
            
            // Get description
            cJSON *desc = cJSON_GetObjectItem(weather, "description");
            if (cJSON_IsString(desc) && (desc->valuestring != NULL)) {
                strlcpy(current_weather.description, desc->valuestring, MAX_WEATHER_DESC_LEN);
                ESP_LOGI(TAG, "Description: %s", current_weather.description);
            }
            
            // Get icon code
            cJSON *icon = cJSON_GetObjectItem(weather, "icon");
            if (cJSON_IsString(icon) && (icon->valuestring != NULL)) {
                strlcpy(current_weather.icon, icon->valuestring, MAX_WEATHER_ICON_LEN);
                current_weather.condition = map_weather_condition(icon->valuestring);
                ESP_LOGI(TAG, "Icon: %s, Condition: %d", current_weather.icon, current_weather.condition);
            }
        }
    }
    
    // Parse hourly forecast
    cJSON *hourly_array = cJSON_GetObjectItem(root, "hourly");
    if (cJSON_IsArray(hourly_array)) {
        int array_size = cJSON_GetArraySize(hourly_array);
        int forecast_count = 0;
        ESP_LOGI(TAG, "Found %d hourly forecasts in API response", array_size);
        ESP_LOGI(TAG, "Current time: %ld, Current hour: %ld", (long)now, (long)current_hour);
        
        // Get forecasts for the next 7 hours
        for (int i = 0; i < array_size && forecast_count < 7; i++) {
            cJSON *hour = cJSON_GetArrayItem(hourly_array, i);
            if (!hour) continue;
            
            // Get timestamp for this hour
            cJSON *dt = cJSON_GetObjectItem(hour, "dt");
            if (!cJSON_IsNumber(dt)) continue;
            
            time_t forecast_time = (time_t)dt->valueint;
            
            // Skip forecasts before our forecast start time
            if (forecast_time < forecast_start) continue;
            
            // Stop if we've gone past our forecast window
            if (forecast_time >= forecast_end) break;
            
            // Store the forecast
            current_weather.hourly[forecast_count].timestamp = forecast_time;
            
            // Get temperature
            cJSON *temp = cJSON_GetObjectItem(hour, "temp");
            if (cJSON_IsNumber(temp)) {
                current_weather.hourly[forecast_count].temperature = temp->valuedouble;
                ESP_LOGI(TAG, "Forecast %d: time=%ld, temp=%.1f", 
                        forecast_count, (long)forecast_time, temp->valuedouble);
            } else {
                current_weather.hourly[forecast_count].temperature = NAN;
                ESP_LOGI(TAG, "Forecast %d: time=%ld, temp=N/A", 
                        forecast_count, (long)forecast_time);
            }
            
            // Get weather info
            cJSON *weather = cJSON_GetObjectItem(hour, "weather");
            if (cJSON_IsArray(weather) && cJSON_GetArraySize(weather) > 0) {
                cJSON *weather_item = cJSON_GetArrayItem(weather, 0);
                if (weather_item) {
                    cJSON *icon = cJSON_GetObjectItem(weather_item, "icon");
                    if (cJSON_IsString(icon) && (icon->valuestring != NULL)) {
                        strlcpy(current_weather.hourly[forecast_count].icon, 
                              icon->valuestring, MAX_WEATHER_ICON_LEN);
                        current_weather.hourly[forecast_count].condition = 
                            map_weather_condition(icon->valuestring);
                    }
                }
            }
            
            forecast_count++;
            
            // No need to check for next day since we're only looking at a 6-hour window
        }
        
        current_weather.hourly_count = forecast_count;
        current_weather.last_forecast_update = now;
        ESP_LOGI(TAG, "Loaded %d hourly forecasts", forecast_count);
    }

    cJSON_Delete(root);
    return ESP_OK;
}

bool weather_client_get_data(weather_data_t *data) {
    if (!data) {
        return false;
    }
    
    // Copy the current weather data to the provided structure
    *data = current_weather;
    return !isnan(data->temperature);
}

bool weather_client_get_description(char *buffer, size_t size) {
    if (!buffer || size == 0) {
        return false;
    }
    
    strlcpy(buffer, current_weather.description, size);
    return strlen(current_weather.description) > 0;
}

bool weather_client_get_icon(char *buffer, size_t size) {
    if (!buffer || size < MAX_WEATHER_ICON_LEN) {
        return false;
    }
    
    strlcpy(buffer, current_weather.icon, size);
    return strlen(current_weather.icon) > 0;
}

weather_condition_t weather_client_get_condition(void) {
    return current_weather.condition;
}

bool weather_client_get_hourly_forecast(uint8_t hour_offset, hourly_forecast_t *forecast) {
    if (!forecast || hour_offset == 0 || hour_offset > current_weather.hourly_count) {
        return false;
    }
    
    // Get the forecast for the requested hour offset (1-based index)
    *forecast = current_weather.hourly[hour_offset - 1];
    return true;
}

uint8_t weather_client_get_hourly_forecast_count(void) {
    return current_weather.hourly_count;
}

time_t weather_client_get_next_forecast_update(void) {
    // Update forecast every 30 minutes
    return current_weather.last_forecast_update + 1800; // 30 minutes in seconds
}

// Initialize the weather client
void weather_client_init(const char *api_key_param, const char *latitude_param, const char *longitude_param) {
    if (api_key_param) {
        strncpy(api_key, api_key_param, MAX_API_KEY_LEN - 1);
        api_key[MAX_API_KEY_LEN - 1] = '\0';
    }
    
    if (latitude_param) {
        strncpy(latitude, latitude_param, MAX_LAT_LON_LEN - 1);
        latitude[MAX_LAT_LON_LEN - 1] = '\0';
    }
    
    if (longitude_param) {
        strncpy(longitude, longitude_param, MAX_LAT_LON_LEN - 1);
        longitude[MAX_LAT_LON_LEN - 1] = '\0';
    }
    
    ESP_LOGI(TAG, "Weather client initialized");
}

// Update weather data from OpenWeatherMap
esp_err_t weather_client_update(void) {
    if (strlen(api_key) == 0 || strlen(latitude) == 0 || strlen(longitude) == 0) {
        ESP_LOGE(TAG, "Weather client not properly initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Build the complete URL with query parameters
    char url[256];
    snprintf(url, sizeof(url), 
             "https://api.openweathermap.org/data/3.0/onecall?lat=%s&lon=%s&exclude=minutely,daily,alerts&units=metric&appid=%s",
             latitude, longitude, api_key);
    
    ESP_LOGI(TAG, "Constructed URL: %s", url);
    
    // Configure HTTP client with the complete URL and timeouts
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .timeout_ms = 15000,  // 15 second timeout
        .buffer_size = 4096,  // Increased buffer size
        .buffer_size_tx = 4096,
        .disable_auto_redirect = false,
        .max_redirection_count = 10,
        .keep_alive_enable = true,
        .keep_alive_idle = 5,
        .keep_alive_interval = 5,
        .keep_alive_count = 3,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    
    // No need to set URL again as it's already set in the config
    esp_err_t err = ESP_OK;
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set URL: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }
    
    // Set HTTP method
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    
    ESP_LOGI(TAG, "Fetching weather data from: %s", url);
    
    // Clear previous response
    if (response_buffer) {
        free(response_buffer);
        response_buffer = NULL;
        response_len = 0;
    }
    
    // Execute request with retry logic
    const int max_retries = 3;
    int retry_count = 0;
    
    do {
        err = esp_http_client_perform(client);
        if (err == ESP_OK || err == ESP_ERR_HTTP_CONNECT) {
            break;  // Success or connection error that won't be fixed by retrying
        }
        
        retry_count++;
        if (retry_count < max_retries) {
            ESP_LOGW(TAG, "Request failed (attempt %d/%d), retrying in 1s...", 
                    retry_count, max_retries);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            
            // Reset the client for retry
            esp_http_client_close(client);
        }
    } while (retry_count < max_retries);
    
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200) {
            if (response_buffer && response_len > 0) {
                // Parse the weather data
                err = parse_weather_data(response_buffer);
            } else {
                ESP_LOGE(TAG, "Empty response from server");
                err = ESP_ERR_INVALID_RESPONSE;
            }
        } else {
            ESP_LOGE(TAG, "HTTP request failed with status: %d", status_code);
            err = ESP_ERR_HTTP_BASE + status_code;
        }
    } else {
        if (err == ESP_ERR_HTTP_CONNECT) {
            ESP_LOGE(TAG, "HTTP connection failed: %s", esp_err_to_name(err));
        } else if (err == ESP_ERR_TIMEOUT) {
            ESP_LOGE(TAG, "HTTP request timed out: %s", esp_err_to_name(err));
        } else if (err == ESP_ERR_HTTP_FETCH_HEADER) {
            ESP_LOGE(TAG, "Failed to fetch HTTP header: %s", esp_err_to_name(err));
        } else {
            ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        }
    }
    
    // Cleanup
    esp_http_client_cleanup(client);
    return err;
}

// Get the current temperature in Celsius
float weather_client_get_temperature(void) {
    return current_weather.temperature;
}
