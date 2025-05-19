#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "weather_client.h"

// Logging tag
static const char *TAG = "WEATHER_CLIENT";

// API configuration
#define WEATHER_API_URL "https://api.openweathermap.org/data/2.5/weather"

// Global variables
static char api_key[MAX_API_KEY_LEN] = {0};
static char latitude[MAX_LAT_LON_LEN] = {0};
static char longitude[MAX_LAT_LON_LEN] = {0};
static float current_temperature = NAN;
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

// Parse temperature from JSON response
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

    // Get the main object which contains temperature
    cJSON *main = cJSON_GetObjectItem(root, "main");
    if (main) {
        cJSON *temp = cJSON_GetObjectItem(main, "temp");
        if (cJSON_IsNumber(temp)) {
            current_temperature = temp->valuedouble - 273.15; // Convert from Kelvin to Celsius
            ESP_LOGI(TAG, "Temperature updated: %.1f°C", current_temperature);
        } else {
            ESP_LOGE(TAG, "Temperature not found in response");
            cJSON_Delete(root);
            return ESP_ERR_NOT_FOUND;
        }
    } else {
        ESP_LOGE(TAG, "'main' object not found in response");
        cJSON_Delete(root);
        return ESP_ERR_NOT_FOUND;
    }

    cJSON_Delete(root);
    return ESP_OK;
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
    
    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = WEATHER_API_URL,
        .event_handler = http_event_handler,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    
    // Build the complete URL with query parameters
    char url[256];
    snprintf(url, sizeof(url), 
             "%s?lat=%s&lon=%s&appid=%s",
             WEATHER_API_URL, latitude, longitude, api_key);
    
    // Set the complete URL
    esp_err_t err = esp_http_client_set_url(client, url);
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
    
    // Execute request
    err = esp_http_client_perform(client);
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
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }
    
    // Cleanup
    esp_http_client_cleanup(client);
    return err;
}

// Get the current temperature in Celsius
float weather_client_get_temperature(void) {
    return current_temperature;
}
