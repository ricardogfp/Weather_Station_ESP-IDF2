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
#define WEATHER_API_URL "https://api.openweathermap.org/data/3.0/onecall?exclude=minutely,alerts&units=metric"

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
    .last_forecast_update = 0,
    .daily_count = 0 // Initialize daily_count
    // .daily array will be zero-initialized by static storage duration
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
                local_buffer[total_len] = '\0';  // Null-terminate the string
                
                // Log the first 256 bytes of the response for debugging
                if (total_len <= 256) {
                    ESP_LOGI(TAG, "Received %d bytes of response data: %.*s", 
                            evt->data_len, evt->data_len, (char*)evt->data);
                }
            }
            break;
            
        case HTTP_EVENT_ON_FINISH:
            // Log the complete response if it's not too large
            if (total_len > 0 && total_len < 2048) {
                ESP_LOGI(TAG, "Complete API response (truncated if >2KB): %.*s", 
                        (int)total_len, local_buffer ? local_buffer : "<null>");
            } else if (total_len > 0) {
                // For larger responses, log just the beginning and end
                ESP_LOGI(TAG, "API response (first 512 bytes): %.*s...", 512, local_buffer);
                ESP_LOGI(TAG, "...API response (last 512 bytes): ...%s", 
                        local_buffer + total_len - 512);
            }
            
            // Allocate memory for the response buffer
            if (response_buffer) {
                free(response_buffer);
            }
            response_buffer = local_buffer;
            response_len = total_len;
            
            // Log the response length for debugging
            ESP_LOGI(TAG, "Response buffer size: %d bytes", total_len);
            
            local_buffer = NULL;
            total_len = 0;
            break;
            
        case HTTP_EVENT_DISCONNECTED:
            if (local_buffer) {
                free(local_buffer);
                local_buffer = NULL;
                total_len = 0;
            }
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
weather_condition_t map_weather_condition(const char *icon) {
    if (!icon || strlen(icon) < 2) {
        return WEATHER_UNKNOWN;
    }

    // First character indicates main weather condition
    switch (icon[0]) {
        case '0': // Group 2xx, 3xx, 5xx, 6xx, 7xx
            return WEATHER_UNKNOWN;
        case '2': // Thunderstorm
            return WEATHER_THUNDERSTORM;
        case '3': // Drizzle
            return WEATHER_RAIN; // Using RAIN for drizzle as it's similar
        case '5': // Rain
            // 10d and 10n are rain, 09d and 09n are shower rain
            if (strcmp(icon, "09d") == 0 || strcmp(icon, "09n") == 0) {
                return WEATHER_SHOWER_RAIN;
            }
            return WEATHER_RAIN;
        case '6': // Snow
            return WEATHER_SNOW;
        case '7': // Atmosphere (Mist, Smoke, Haze, etc.)
            return WEATHER_MIST;
        case '8': // Clouds
            // Check second character for cloudiness
            if (icon[1] == '0') return WEATHER_CLEAR_SKY; // 01d/n
            if (icon[1] == '1' || icon[1] == '2') return WEATHER_FEW_CLOUDS; // 02d/n, 02n
            if (icon[1] == '3' || icon[1] == '4') return WEATHER_BROKEN_CLOUDS; // 03d/n, 04d/n
            return WEATHER_BROKEN_CLOUDS;
        case '9': // Additional conditions (extreme)
            return WEATHER_UNKNOWN; // No extreme condition in our enum
        default:
            ESP_LOGW(TAG, "Unknown weather icon code: %s", icon);
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
        ESP_LOGE(TAG, "Failed to parse JSON: %s", cJSON_GetErrorPtr());
        return ESP_FAIL;
    }

    // Get current time for forecast window calculation
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char current_time_str[20];
    strftime(current_time_str, sizeof(current_time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "Current time: %s", current_time_str);

    // Calculate forecast window (e.g., next 7 hours for hourly)
    time_t forecast_end_time = now + (MAX_HOURLY_FORECAST_DISPLAY * 3600); // Example: 7 hours window
    struct tm forecast_end_timeinfo;
    localtime_r(&forecast_end_time, &forecast_end_timeinfo);
    char forecast_end_time_str[20];
    strftime(forecast_end_time_str, sizeof(forecast_end_time_str), "%Y-%m-%d %H:%M:%S", &forecast_end_timeinfo);
    ESP_LOGI(TAG, "Forecast window: %s to %s", current_time_str, forecast_end_time_str);

    // Parse current weather
    cJSON *current = cJSON_GetObjectItem(root, "current");
    if (current) {
        current_weather.temperature = cJSON_GetObjectItem(current, "temp") ? 
                                      (float)cJSON_GetObjectItem(current, "temp")->valuedouble : NAN;
        ESP_LOGI(TAG, "Current temperature: %.1f°C", current_weather.temperature);

        cJSON *weather_array = cJSON_GetObjectItem(current, "weather");
        if (cJSON_IsArray(weather_array) && cJSON_GetArraySize(weather_array) > 0) {
            cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);
            const char *desc = cJSON_GetObjectItem(weather_item, "description") ? 
                               cJSON_GetObjectItem(weather_item, "description")->valuestring : "N/A";
            strncpy(current_weather.description, desc, MAX_WEATHER_DESC_LEN - 1);
            current_weather.description[MAX_WEATHER_DESC_LEN - 1] = '\0';
            ESP_LOGI(TAG, "Description: %s", current_weather.description);

            const char *icon_code = cJSON_GetObjectItem(weather_item, "icon") ? 
                                    cJSON_GetObjectItem(weather_item, "icon")->valuestring : "01d";
            strncpy(current_weather.icon, icon_code, MAX_WEATHER_ICON_LEN - 1);
            current_weather.icon[MAX_WEATHER_ICON_LEN - 1] = '\0';
            current_weather.condition = map_weather_condition(current_weather.icon);
            ESP_LOGI(TAG, "Icon: %s, Condition: %d", current_weather.icon, current_weather.condition);
        }
    }

    // Parse hourly forecast
    cJSON *hourly_array = cJSON_GetObjectItem(root, "hourly");
    if (cJSON_IsArray(hourly_array)) {
        int num_hourly = cJSON_GetArraySize(hourly_array);
        ESP_LOGI(TAG, "Found %d hourly forecasts in API response", num_hourly);
        current_weather.hourly_count = 0;
        time_t current_timestamp;
        time(&current_timestamp); // Get current time to filter out past forecasts
        ESP_LOGI(TAG, "Current timestamp: %ld", (long)current_timestamp);

        for (int i = 0; i < num_hourly && current_weather.hourly_count < MAX_HOURLY_FORECAST; ++i) {
            cJSON *item = cJSON_GetArrayItem(hourly_array, i);
            time_t forecast_time = cJSON_GetObjectItem(item, "dt") ? 
                                   (time_t)cJSON_GetObjectItem(item, "dt")->valueint : 0;

            // Only consider forecasts that are in the future and within the display window (e.g., next 7 hours)
            // The API gives 48 hours, we might only want to store/display a subset.
            if (forecast_time >= current_timestamp && forecast_time <= forecast_end_time) {
                current_weather.hourly[current_weather.hourly_count].timestamp = forecast_time;
                current_weather.hourly[current_weather.hourly_count].temperature = 
                    cJSON_GetObjectItem(item, "temp") ? (float)cJSON_GetObjectItem(item, "temp")->valuedouble : NAN;
                
                ESP_LOGI(TAG, "Forecast %d: time=%ld, temp=%.1f", current_weather.hourly_count,
                         (long)current_weather.hourly[current_weather.hourly_count].timestamp,
                         current_weather.hourly[current_weather.hourly_count].temperature);

                cJSON *weather_array_hourly = cJSON_GetObjectItem(item, "weather");
                if (cJSON_IsArray(weather_array_hourly) && cJSON_GetArraySize(weather_array_hourly) > 0) {
                    cJSON *weather_item_hourly = cJSON_GetArrayItem(weather_array_hourly, 0);
                    const char *icon_code_hourly = cJSON_GetObjectItem(weather_item_hourly, "icon") ? 
                                                   cJSON_GetObjectItem(weather_item_hourly, "icon")->valuestring : "01d";
                    strncpy(current_weather.hourly[current_weather.hourly_count].icon, 
                            icon_code_hourly, MAX_WEATHER_ICON_LEN - 1);
                    current_weather.hourly[current_weather.hourly_count].icon[MAX_WEATHER_ICON_LEN - 1] = '\0';
                    current_weather.hourly[current_weather.hourly_count].condition = 
                        map_weather_condition(current_weather.hourly[current_weather.hourly_count].icon);
                }
                current_weather.hourly_count++;
            }
        }
        ESP_LOGI(TAG, "Loaded %d hourly forecasts", current_weather.hourly_count);
    }

    // Parse daily forecast
    cJSON *daily_array = cJSON_GetObjectItem(root, "daily");
    if (cJSON_IsArray(daily_array)) {
        current_weather.daily_count = 0; // Reset count before parsing
        int num_daily = cJSON_GetArraySize(daily_array);
        ESP_LOGI(TAG, "Daily forecast items in JSON: %d", num_daily);
        for (int i = 0; i < num_daily && current_weather.daily_count < MAX_DAILY_FORECAST; ++i) {
            cJSON *item = cJSON_GetArrayItem(daily_array, i);
            if (!item) continue;

            current_weather.daily[current_weather.daily_count].timestamp =
                cJSON_GetObjectItem(item, "dt") ? (time_t)cJSON_GetObjectItem(item, "dt")->valueint : 0;

            cJSON *temp_obj = cJSON_GetObjectItem(item, "temp");
            if (temp_obj) {
                current_weather.daily[current_weather.daily_count].temp_min =
                    cJSON_GetObjectItem(temp_obj, "min") ? cJSON_GetObjectItem(temp_obj, "min")->valuedouble : NAN;
                current_weather.daily[current_weather.daily_count].temp_max =
                    cJSON_GetObjectItem(temp_obj, "max") ? cJSON_GetObjectItem(temp_obj, "max")->valuedouble : NAN;
            } else {
                current_weather.daily[current_weather.daily_count].temp_min = NAN;
                current_weather.daily[current_weather.daily_count].temp_max = NAN;
            }

            cJSON *weather_array_daily = cJSON_GetObjectItem(item, "weather");
            if (cJSON_IsArray(weather_array_daily) && cJSON_GetArraySize(weather_array_daily) > 0) {
                cJSON *weather_item_daily = cJSON_GetArrayItem(weather_array_daily, 0);
                const char *icon_str = cJSON_GetObjectItem(weather_item_daily, "icon") ? cJSON_GetObjectItem(weather_item_daily, "icon")->valuestring : "";
                strncpy(current_weather.daily[current_weather.daily_count].icon, icon_str, sizeof(current_weather.daily[current_weather.daily_count].icon) - 1);
                current_weather.daily[current_weather.daily_count].icon[sizeof(current_weather.daily[current_weather.daily_count].icon) - 1] = '\0';
            } else {
                current_weather.daily[current_weather.daily_count].icon[0] = '\0';
            }
            ESP_LOGD(TAG, "Daily[%d]: ts=%ld, min=%.1f, max=%.1f, icon=%s", 
                current_weather.daily_count,
                (long)current_weather.daily[current_weather.daily_count].timestamp,
                current_weather.daily[current_weather.daily_count].temp_min,
                current_weather.daily[current_weather.daily_count].temp_max,
                current_weather.daily[current_weather.daily_count].icon);
            current_weather.daily_count++;
        }
    } else {
        ESP_LOGW(TAG, "Daily forecast data not found or not an array.");
        current_weather.daily_count = 0;
    }

    // After parsing daily forecasts, if today's forecast (daily[0]) is available,
    // use its min/max for the current_weather's main temp_min and temp_max.
    if (current_weather.daily_count > 0) {
        current_weather.temp_min = current_weather.daily[0].temp_min;
        current_weather.temp_max = current_weather.daily[0].temp_max;
        ESP_LOGI(TAG, "Updated current_weather.temp_min/max from daily[0]: %.1f°C / %.1f°C", 
                 current_weather.temp_min, current_weather.temp_max);
    } else {
        // If no daily forecast, ensure current_weather.temp_min/max are NAN or some default
        // (They might have been set by the 'current' weather section if that's preferred as a fallback)
        // For now, we assume if daily[0] is not there, we don't override what 'current' might have set.
        // If 'current' doesn't set them, they remain as initialized (likely NAN or 0).
        ESP_LOGW(TAG, "No daily forecast data to update current_weather.temp_min/max.");
    }

    current_weather.last_forecast_update = now;
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
    if (hour_offset < current_weather.hourly_count && forecast) {
        *forecast = current_weather.hourly[hour_offset];
        return true;
    }
    return false;
}

uint8_t weather_client_get_hourly_forecast_count(void) {
    return current_weather.hourly_count;
}

bool weather_client_get_daily_forecast(uint8_t day_offset, daily_forecast_t *forecast) {
    if (day_offset < current_weather.daily_count && forecast) {
        *forecast = current_weather.daily[day_offset];
        return true;
    }
    return false;
}

uint8_t weather_client_get_daily_forecast_count(void) {
    return current_weather.daily_count;
}

time_t weather_client_get_next_forecast_update(void) {
    // Update forecast every 30 minutes
    return current_weather.last_forecast_update + 1800; // 30 minutes in seconds
}

void init_weather_client(void) {
    memset(&current_weather, 0, sizeof(current_weather));
    current_weather.temperature = NAN;
    current_weather.temp_min = NAN; 
    current_weather.temp_max = NAN; 
    current_weather.last_forecast_update = 0;
    current_weather.hourly_count = 0;
    current_weather.daily_count = 0;
    // No need to initialize arrays current_weather.hourly and current_weather.daily here as memset did it.
}

// Initialize the weather client
void weather_client_init(const char *api_key_param, const char *latitude_param, const char *longitude_param) {
    init_weather_client();
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
             "https://api.openweathermap.org/data/3.0/onecall?lat=%s&lon=%s&exclude=minutely,alerts&units=metric&appid=%s",
             latitude, longitude, api_key);
    
    ESP_LOGI(TAG, "Fetching weather data from: %s", url);
    
    // Initialize HTTP client configuration
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .timeout_ms = 15000,  // 15 second timeout
        .buffer_size = 8192,  // Increased buffer size for larger responses
        .buffer_size_tx = 2048,
        .disable_auto_redirect = false,
        .max_redirection_count = 10,
        .keep_alive_enable = true,
        .keep_alive_idle = 5,
        .keep_alive_interval = 5,
        .keep_alive_count = 3,
    };
    
    ESP_LOGI(TAG, "Initializing HTTP client...");
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    
    // Set headers
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Accept", "application/json");
    
    // Clear previous response if any
    if (response_buffer) {
        free(response_buffer);
        response_buffer = NULL;
        response_len = 0;
    }
    
    ESP_LOGI(TAG, "Sending HTTP GET request...");
    
    // Perform the request with retry logic
    const int max_retries = 3;
    int retry_count = 0;
    esp_err_t err;
    
    do {
        err = esp_http_client_perform(client);
        
        // Get status code and content length
        int status_code = esp_http_client_get_status_code(client);
        int content_length = esp_http_client_get_content_length(client);
        
        ESP_LOGI(TAG, "HTTP Status: %d, Content-Length: %d, Error: %s", 
                status_code, content_length, esp_err_to_name(err));
        
        if (err == ESP_OK) {
            if (status_code == 200) {
                // Parse the response if we got one
                if (response_buffer && response_len > 0) {
                    ESP_LOGI(TAG, "Parsing weather data (%.*s...)", 
                            response_len > 100 ? 100 : response_len, response_buffer);
                    
                    esp_err_t parse_err = parse_weather_data(response_buffer);
                    if (parse_err != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to parse weather data");
                        esp_http_client_cleanup(client);
                        return parse_err;
                    }
                    ESP_LOGI(TAG, "Successfully parsed weather data");
                    break;
                } else {
                    ESP_LOGE(TAG, "Empty response from server");
                    err = ESP_ERR_INVALID_RESPONSE;
                }
            } else {
                // Log detailed error response if available
                if (response_buffer && response_len > 0) {
                    ESP_LOGE(TAG, "API Error Response: %.*s", response_len, response_buffer);
                }
                ESP_LOGE(TAG, "HTTP request failed with status: %d, error: %s", 
                        status_code, esp_err_to_name(err));
                err = ESP_ERR_HTTP_BASE + status_code;
            }
        }
        
        // If we get here, there was an error
        retry_count++;
        if (retry_count < max_retries) {
            ESP_LOGW(TAG, "Request failed (attempt %d/%d), retrying in 1s...", 
                    retry_count, max_retries);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            
            // Reset the client for retry
            esp_http_client_close(client);
        } else {
            // Log the final error
            const char *error_name = esp_err_to_name(err);
            int http_errno = esp_http_client_get_errno(client);
            
            if (err == ESP_ERR_HTTP_CONNECT) {
                ESP_LOGE(TAG, "HTTP connection failed: %s (errno: %d)", error_name, http_errno);
            } else if (err == ESP_ERR_TIMEOUT) {
                ESP_LOGE(TAG, "HTTP request timed out: %s (errno: %d)", error_name, http_errno);
            } else if (err == ESP_ERR_HTTP_FETCH_HEADER) {
                ESP_LOGE(TAG, "Failed to fetch HTTP header: %s (errno: %d)", error_name, http_errno);
            } else if (err == ESP_ERR_HTTP_EAGAIN) {
                ESP_LOGE(TAG, "HTTP EAGAIN error, try again: %s (errno: %d)", error_name, http_errno);
            } else {
                ESP_LOGE(TAG, "HTTP request failed: %s (errno: %d)", error_name, http_errno);
            }
            
            // Log the response if we got any
            if (response_buffer && response_len > 0) {
                ESP_LOGE(TAG, "Response data (first 256 bytes): %.*s", 
                        response_len > 256 ? 256 : response_len, response_buffer);
            }
            
            break;
        }
    } while (retry_count < max_retries);
    
    // Cleanup
    esp_http_client_cleanup(client);
    
    // Log memory stats
    ESP_LOGI(TAG, "Free heap after request: %d bytes", (int)esp_get_free_heap_size());
    
    return err;
}
