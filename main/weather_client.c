#include <string.h>
#include <time.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "weather_client.h"
#include "UI/images.h"

// Logging tag
static const char *TAG = "WEATHER_CLIENT";

// Buffer sizes
#define MAX_HTTP_OUTPUT_BUFFER 16384
#define CHUNK_BUFFER_SIZE 4096

// Weather API configuration
#define WEATHER_API_URL "https://api.openweathermap.org/data/3.0/onecall"
//#define WEATHER_API_KEY "b2c0beef44dd157da425e740c95d37d0"  // Replace with your actual API key

// Default location (can be overridden)
#define DEFAULT_LATITUDE 40.4168f  // Madrid, Spain
#define DEFAULT_LONGITUDE -3.7038f

// Global weather data
WeatherData currentWeather = {0};
HourlyWeather hourlyWeather = {0};
DailyWeather dailyWeather = {0};

// Weather icons and descriptions
char str_Current_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_Weather_Description[MAX_WEATHER_DESC_LEN] = "Clear sky";
char str_1h_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_2h_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_3h_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_4h_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_5h_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_6h_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_7h_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_1d_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_2d_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_3d_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_4d_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_5d_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_6d_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";
char str_7d_weather_icon[MAX_WEATHER_ICON_LEN] = "01d";

// HTTP client state
static char *response_buffer = NULL;
static size_t total_len = 0;

// External configuration defined in main.c
extern const char *openWeatherMapApiKey;
extern const char *lat;
extern const char *lon;

// Status information
char ca_Info_Status[32] = "Online";

// Buffer for storing JSON response from OpenWeatherMap
static char jsonBuffer[WEATHER_JSON_BUFFER_SIZE];

// Task handle and status
static TaskHandle_t weather_task_handle = NULL;
static bool weather_task_running = false;

// Forward declarations
static esp_err_t http_event_handler(esp_http_client_event_t *evt);
static void parse_weather_data(const char *json_str);
static esp_err_t fetch_weather_data(void);

// Check if WiFi is connected
bool wifi_is_connected(void) {
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    return (ret == ESP_OK);
}

// Initialize the weather client
void init_weather_client(void) {
    // Initialize with default values
    currentWeather.temperature = 20.0;
    currentWeather.temp_max = 25.0;
    currentWeather.temp_min = 15.0;
    
    // Initialize hourly weather data
    for (int i = 0; i < MAX_HOURS; i++) {
        hourlyWeather.hours[i].temperature = 20.0 + (i % 5);
        hourlyWeather.hours[i].temp_max = 25.0 + (i % 5);
        hourlyWeather.hours[i].temp_min = 15.0 + (i % 5);
    }
    
    // Initialize daily weather data
    for (int i = 0; i < MAX_DAYS; i++) {
        dailyWeather.days[i].temperature = 20.0 + (i % 7);
        dailyWeather.days[i].temp_max = 25.0 + (i % 7);
        dailyWeather.days[i].temp_min = 15.0 + (i % 7);
    }
    
    ESP_LOGI(TAG, "Weather client initialized");
}

// HTTP client event handler (used with ESP HTTP Client)

// HTTP event handler
static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static char *response_buffer = NULL;
    static size_t total_len = 0;
    static bool is_chunked = false;
    static size_t content_length = 0;
    static bool response_complete = false;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            response_complete = false;
            break;
            
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER: %.*s", evt->data_len, (char*)evt->data);
            
            // Check for chunked transfer
            if (strncasecmp(evt->data, "Transfer-Encoding: chunked", evt->data_len) == 0) {
                is_chunked = true;
                ESP_LOGD(TAG, "Chunked transfer encoding detected");
            }
            // Get content length if available
            else if (strncasecmp(evt->data, "Content-Length: ", 16) == 0) {
                content_length = atoi((char*)evt->data + 16);
                ESP_LOGD(TAG, "Content length: %d bytes", content_length);
            }
            break;

        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA: len=%d", evt->data_len);
            
            if (response_buffer == NULL) {
                // Allocate buffer with some extra space
                size_t buffer_size = is_chunked ? CHUNK_BUFFER_SIZE : 
                                     (content_length > 0 ? content_length + 1 : MAX_HTTP_OUTPUT_BUFFER);
                response_buffer = (char *)malloc(buffer_size);
                if (response_buffer == NULL) {
                    ESP_LOGE(TAG, "Failed to allocate %d bytes for response buffer", buffer_size);
                    return ESP_FAIL;
                }
                total_len = 0;
                ESP_LOGD(TAG, "Allocated %d bytes for response buffer", buffer_size);
            }

            // Check if we have enough space
            if (total_len + evt->data_len < MAX_HTTP_OUTPUT_BUFFER - 1) {
                memcpy(response_buffer + total_len, evt->data, evt->data_len);
                total_len += evt->data_len;
                response_buffer[total_len] = '\0'; // Ensure null termination
            } else {
                ESP_LOGE(TAG, "Response too large, discarding");
                free(response_buffer);
                response_buffer = NULL;
                total_len = 0;
                return ESP_FAIL;
            }
            break;
            
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (response_buffer && total_len > 0) {
                response_buffer[total_len] = '\0'; // Ensure null termination
                ESP_LOGD(TAG, "Response complete, %d bytes", total_len);
                parse_weather_data(response_buffer);
                response_complete = true;
            } else {
                ESP_LOGE(TAG, "No data received in response");
            }
            break;

        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            if (!response_complete && response_buffer && total_len > 0) {
                // If we got disconnected before finishing, try to parse what we have
                response_buffer[total_len] = '\0';
                ESP_LOGW(TAG, "Incomplete response received, attempting to parse %d bytes", total_len);
                parse_weather_data(response_buffer);
            }
            // Fall through to cleanup
            
        case HTTP_EVENT_ERROR:
            if (evt->event_id == HTTP_EVENT_ERROR) {
                ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            }
            // Clean up resources
            if (response_buffer) {
                free(response_buffer);
                response_buffer = NULL;
            }
            total_len = 0;
            content_length = 0;
            is_chunked = false;
            response_complete = false;
            break;

        default:
            break;
    }
    return ESP_OK;
}

// Dedicated task for weather data fetching

/**
 * @brief Weather fetch task that runs in its own task context
 */
static void weather_fetch_task(void *pvParameters) {
    ESP_LOGI(TAG, "Weather fetch task started");
    
    // Wait for WiFi to be connected
    while (!wifi_is_connected()) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Fetch weather data
    esp_err_t err = fetch_weather_data();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to fetch weather data");
    }
    
    ESP_LOGI(TAG, "Weather fetch task ending with %d bytes free stack", uxTaskGetStackHighWaterMark(NULL));
    weather_task_running = false;
    vTaskDelete(NULL);
}

// Function to fetch weather data from OpenWeatherMap API
static esp_err_t fetch_weather_data(void) {
    char url[256];
    snprintf(url, sizeof(url), "%s?lat=%.4f&lon=%.4f&exclude=minutely,alerts&units=metric&appid=%s",
             WEATHER_API_URL, DEFAULT_LATITUDE, DEFAULT_LONGITUDE, openWeatherMapApiKey);
    
    ESP_LOGI(TAG, "Using API URL: %s", url);
    
    // Configure the HTTP client with the URL
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .buffer_size = 16384,  // Increased buffer size for large JSON responses
        .buffer_size_tx = 4096,
        .timeout_ms = 60000,   // 60 seconds timeout for slow connections
        .disable_auto_redirect = false,
        .max_redirection_count = 10,
        .keep_alive_enable = true,
        .keep_alive_idle = 5,     // 5 seconds keepalive idle time
        .keep_alive_interval = 5,  // 5 seconds between keepalive packets
        .keep_alive_count = 3,     // 3 keepalive packets before giving up
        .skip_cert_common_name_check = true,  // Skip CN check for HTTPS
        .user_agent = "ESP32 Weather Station"
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    
    // Set headers
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Accept", "application/json");
    
    // Perform the request
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }
    
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP request completed, status = %d, content_length = %lld",
            status_code,
            esp_http_client_get_content_length(client));
    
    // Clean up
    esp_http_client_cleanup(client);
    return (status_code == 200) ? ESP_OK : ESP_FAIL;
}

// Function to parse weather data from JSON response
static void parse_weather_data(const char *json_str) {
    if (!json_str || *json_str == '\0') {
        ESP_LOGE(TAG, "Empty JSON string received");
        return;
    }

    ESP_LOGI(TAG, "Parsing weather data...");
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON: %s", cJSON_GetErrorPtr());
        return;
    }

    // Parse current weather
    cJSON *current = cJSON_GetObjectItem(root, "current");
    if (current) {
        // Parse temperature
        cJSON *temp = cJSON_GetObjectItem(current, "temp");
        if (cJSON_IsNumber(temp)) {
            currentWeather.temperature = temp->valuedouble;
            ESP_LOGI(TAG, "Current temperature: %.1f°C", currentWeather.temperature);
        }

        // Parse weather condition and icon
        cJSON *weather = cJSON_GetObjectItem(current, "weather");
        if (cJSON_IsArray(weather) && cJSON_GetArraySize(weather) > 0) {
            cJSON *weather_item = cJSON_GetArrayItem(weather, 0);
            if (weather_item) {
                // Get weather icon
                cJSON *icon = cJSON_GetObjectItem(weather_item, "icon");
                if (cJSON_IsString(icon) && (icon->valuestring != NULL)) {
                    strncpy(str_Current_weather_icon, icon->valuestring, MAX_WEATHER_ICON_LEN - 1);
                    str_Current_weather_icon[MAX_WEATHER_ICON_LEN - 1] = '\0';
                    ESP_LOGI(TAG, "Current weather icon: %s", str_Current_weather_icon);
                }
                
                // Get weather description
                cJSON *desc = cJSON_GetObjectItem(weather_item, "description");
                if (cJSON_IsString(desc) && (desc->valuestring != NULL)) {
                    strncpy(str_Weather_Description, desc->valuestring, MAX_WEATHER_DESC_LEN - 1);
                    str_Weather_Description[MAX_WEATHER_DESC_LEN - 1] = '\0';
                    ESP_LOGI(TAG, "Weather description: %s", str_Weather_Description);
                }
            }
        }
    }

    // Parse hourly forecast (next 7 hours)
    cJSON *hourly = cJSON_GetObjectItem(root, "hourly");
    if (cJSON_IsArray(hourly)) {
        int count = cJSON_GetArraySize(hourly);
        int max_hours = (count < 7) ? count : 7;
        
        for (int i = 0; i < max_hours; i++) {
            cJSON *hour = cJSON_GetArrayItem(hourly, i);
            if (hour) {
                cJSON *weather = cJSON_GetObjectItem(hour, "weather");
                if (cJSON_IsArray(weather) && cJSON_GetArraySize(weather) > 0) {
                    cJSON *weather_item = cJSON_GetArrayItem(weather, 0);
                    if (weather_item) {
                        cJSON *icon = cJSON_GetObjectItem(weather_item, "icon");
                        if (cJSON_IsString(icon) && (icon->valuestring != NULL)) {
                            char *target_icon = NULL;
                            switch(i) {
                                case 0: target_icon = str_1h_weather_icon; break;
                                case 1: target_icon = str_2h_weather_icon; break;
                                case 2: target_icon = str_3h_weather_icon; break;
                                case 3: target_icon = str_4h_weather_icon; break;
                                case 4: target_icon = str_5h_weather_icon; break;
                                case 5: target_icon = str_6h_weather_icon; break;
                                case 6: target_icon = str_7h_weather_icon; break;
                            }
                            if (target_icon) {
                                strncpy(target_icon, icon->valuestring, MAX_WEATHER_ICON_LEN - 1);
                                target_icon[MAX_WEATHER_ICON_LEN - 1] = '\0';
                                ESP_LOGI(TAG, "Hour %d weather icon updated to: %s", i, target_icon);
                            }
                        }
                    }
                }
            }
        }
    }

    // Parse daily forecast (next 7 days)
    cJSON *daily = cJSON_GetObjectItem(root, "daily");
    if (cJSON_IsArray(daily)) {
        int count = cJSON_GetArraySize(daily);
        int max_days = (count < 7) ? count : 7;
        
        for (int i = 0; i < max_days; i++) {
            cJSON *day = cJSON_GetArrayItem(daily, i);
            if (day) {
                cJSON *weather = cJSON_GetObjectItem(day, "weather");
                if (cJSON_IsArray(weather) && cJSON_GetArraySize(weather) > 0) {
                    cJSON *weather_item = cJSON_GetArrayItem(weather, 0);
                    if (weather_item) {
                        cJSON *icon = cJSON_GetObjectItem(weather_item, "icon");
                        if (cJSON_IsString(icon) && (icon->valuestring != NULL)) {
                            char *target_icon = NULL;
                            switch(i) {
                                case 0: target_icon = str_1d_weather_icon; break;
                                case 1: target_icon = str_2d_weather_icon; break;
                                case 2: target_icon = str_3d_weather_icon; break;
                                case 3: target_icon = str_4d_weather_icon; break;
                                case 4: target_icon = str_5d_weather_icon; break;
                                case 5: target_icon = str_6d_weather_icon; break;
                                case 6: target_icon = str_7d_weather_icon; break;
                            }
                            if (target_icon) {
                                strncpy(target_icon, icon->valuestring, MAX_WEATHER_ICON_LEN - 1);
                                target_icon[MAX_WEATHER_ICON_LEN - 1] = '\0';
                                ESP_LOGI(TAG, "Day %d weather icon updated to: %s", i, target_icon);
                            }
                        }
                    }
                }
            }
        }
    }

    cJSON_Delete(root);
}

/**
 * @brief Initiates the weather data fetch process
 * @return ESP_OK if task was created successfully, otherwise an error
 */
esp_err_t get_data_from_openweathermap(void) {
    // Don't start a new task if one is already running
    if (weather_task_running) {
        ESP_LOGW(TAG, "Weather fetch task already running");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Create a task with sufficient stack size to fetch the weather data
    weather_task_running = true;
    BaseType_t result = xTaskCreate(
        weather_fetch_task,       // Task function
        "weather_fetch_task",    // Task name
        12288,                   // Stack size in words (12K, increased to prevent overflow)
        NULL,                    // No parameters
        tskIDLE_PRIORITY + 1,    // Priority
        &weather_task_handle     // Task handle
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create weather fetch task");
        weather_task_running = false;
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Weather fetch task created");
    return ESP_OK;
}

/**
 * @brief Get weather icon code for the current weather
 * @return const char* Current weather icon code
 */
const char* get_weather_icon_code_current(void) {
    return str_Current_weather_icon;
}

// Function to get hourly forecast icon code
const char* get_weather_icon_code_hourly(int hour_index) {
    switch (hour_index) {
        case 0: return str_1h_weather_icon;
        case 1: return str_2h_weather_icon;
        case 2: return str_3h_weather_icon;
        case 3: return str_4h_weather_icon;
        case 4: return str_5h_weather_icon;
        case 5: return str_6h_weather_icon;
        case 6: return str_7h_weather_icon;
        default: return "01d";
    }
}

/**
 * @brief Get weather icon code for the daily weather by index
 * @param day_index Day index (0-6)
 * @return Daily weather icon code
 */
const char* get_weather_icon_code_daily(int day_index) {
    switch (day_index) {
        case 0: return str_1d_weather_icon;
        case 1: return str_2d_weather_icon;
        case 2: return str_3d_weather_icon;
        case 3: return str_4d_weather_icon;
        case 4: return str_5d_weather_icon;
        case 5: return str_6d_weather_icon;
        case 6: return str_7d_weather_icon;
        default: return "01d";
    }
}

// Function to get future hour string
const char* get_future_hour(int hours_ahead) {
    static char time_str[6]; // HH:MM\0
    time_t now;
    struct tm timeinfo;
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    // Add hours
    timeinfo.tm_hour += hours_ahead;
    mktime(&timeinfo); // Normalize the time
    
    strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);
    return time_str;
}

// Function to get future date string
const char* get_future_date(int days_ahead) {
    static char date_str[11]; // YYYY-MM-DD\0
    time_t now;
    struct tm timeinfo;
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    // Add days
    timeinfo.tm_mday += days_ahead;
    mktime(&timeinfo); // Normalize the date
    
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", &timeinfo);
    return date_str;
}

// Function to format temperature with degree symbol
const char* format_temp(const char* temp) {
    static char temp_str[10];
    if (temp && temp[0] != '\0') {
        snprintf(temp_str, sizeof(temp_str), "%s°C", temp);
    } else {
        strcpy(temp_str, "N/A");
    }
    return temp_str;
}