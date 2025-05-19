/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "secrets.h"
// Suppress unused variable warnings
#define SUPPRESS_UNUSED_WARNING(x) (void)(x)
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "lvgl.h"

#include "waveshare_rgb_lcd_port.h"
#include "ds3231.h"
#include "wifi_manager.h"
#include "UI/ui.h"
#include "UI/screens.h"
#include "weather_client.h"
#include "cJSON.h"
#include "UI/images.h" // Include the images header for weather icons

// Tag for logging
static const char *CLOCK_TAG = "clock_example";

// Function to get the appropriate icon for a weather code
static const void* get_icon_for_code(const char* code) {
    if (!code || strlen(code) < 3) {
        ESP_LOGW(CLOCK_TAG, "Invalid weather icon code, using default");
        return &img_icon_01d_72p; // fallback to clear day
    }

    if (strcmp(code, "01d") == 0) return &img_icon_01d_72p;
    if (strcmp(code, "01n") == 0) return &img_icon_01n_72p;
    if (strcmp(code, "02d") == 0) return &img_icon_02d_72p;
    if (strcmp(code, "02n") == 0) return &img_icon_02n_72p;
    if (strcmp(code, "03d") == 0 || strcmp(code, "03n") == 0) return &img_icon_03d_03n_72p;
    if (strcmp(code, "04d") == 0 || strcmp(code, "04n") == 0) return &img_icon_04d_04n_72p;
    if (strcmp(code, "09d") == 0 || strcmp(code, "09n") == 0) return &img_icon_09d_09n_72p;
    if (strcmp(code, "10d") == 0) return &img_icon_10d_72p;
    if (strcmp(code, "10n") == 0) return &img_icon_10n_72p;
    if (strcmp(code, "11d") == 0 || strcmp(code, "11n") == 0) return &img_icon_11d_11n_72p;
    if (strcmp(code, "13d") == 0 || strcmp(code, "13n") == 0) return &img_icon_13d_13n_72p;
    if (strcmp(code, "50d") == 0 || strcmp(code, "50n") == 0) return &img_icon_50d_50n_72p;

    ESP_LOGW(CLOCK_TAG, "Unknown weather code: %s, using fallback icon", code);
    return &img_icon_01d_72p; // fallback to clear day
}

// Function to set weather icon on an LVGL image object
static void set_weather_icon(const char* code, lv_obj_t* imgObj) {
    if (!imgObj) {
        ESP_LOGE(CLOCK_TAG, "set_weather_icon: imgObj is NULL");
        return;
    }
    
    // Default to clear day if no valid code
    const char* icon_code = (code && strlen(code) >= 3) ? code : "01d";
    
    ESP_LOGI(CLOCK_TAG, "Setting weather icon for code: %s", icon_code);
    const void* icon = get_icon_for_code(icon_code);
    
    if (!icon) {
        ESP_LOGE(CLOCK_TAG, "Failed to get icon for code: %s, using default", icon_code);
        icon = get_icon_for_code("01d");
    }
    
    // Set the new icon directly - no need to clear first with LVGL
    lv_img_set_src(imgObj, icon);
    
    // Force a screen refresh
    lv_refr_now(NULL);
    
    ESP_LOGI(CLOCK_TAG, "Icon set successfully for code: %s", icon_code);
}

// OpenWeatherMap configuration
static const char *openWeatherMapApiKey = OPENWEATHER_API_KEY;
static const char *lat = OPENWEATHER_LAT;
static const char *lon = OPENWEATHER_LON;

// Initialize weather client
static void init_weather_client(void) {
    weather_client_init(openWeatherMapApiKey, lat, lon);
    ESP_LOGI(CLOCK_TAG, "Weather client initialized with lat: %s, lon: %s", lat, lon);
}

// WiFi credentials
const char *wifi_ssid = WIFI_SSID;
const char *wifi_password = WIFI_PASSWORD;

// Home Assistant configuration
const char *HOME_ASSISTANT_URL = HA_URL;
const char *HOME_ASSISTANT_TOKEN = HA_TOKEN;

// Flag indicating time source
#define TIME_SOURCE_INTERNAL 0
#define TIME_SOURCE_RTC      1
#define TIME_SOURCE_NTP      2
static int time_source = TIME_SOURCE_INTERNAL;

// Day and month names for the UI
static int d_year, d_month, d_day, t_hour, t_minute, t_second;
static int daysOfTheWeek_Val;
static const char *daysOfTheWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char *namesOfMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Weather update timer
static lv_timer_t *weather_update_timer = NULL;

// Function prototypes
static void update_time_display(void);
static void create_weather_ui(void);
static void update_weather_data(void);
static void get_date_time(void);
static void weather_update_timer_cb(lv_timer_t *timer);
static void time_update_timer_cb(lv_timer_t *timer);

/**
 * @brief// IP address acquisition event handler for WiFi
 */
static void wifi_got_ip_event_handler(void* arg, esp_event_base_t event_base,
                                         int32_t event_id, void* event_data)
{
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(CLOCK_TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
    
    // Now that we have an IP address, try to synchronize time with NTP
    ESP_LOGI(CLOCK_TAG, "Starting NTP synchronization");
    
    // Try NTP sync
    if (sntp_init_sync() == ESP_OK) {
        // Get current time
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Check if time is set correctly
        if (timeinfo.tm_year > (2020 - 1900)) {
            time_source = TIME_SOURCE_NTP;
            ESP_LOGI(CLOCK_TAG, "Time synchronized with NTP");
            
            // Update RTC with current time if available
            if (ds3231_init() == ESP_OK) {
                ds3231_write_time(now);
                ESP_LOGI(CLOCK_TAG, "RTC updated with NTP time");
            }
            
            // Update display with new time
            update_time_display();
            
            // Now that we have WiFi and time is synchronized, update weather data
            ESP_LOGI(CLOCK_TAG, "Triggering initial weather data update");
            update_weather_data();
        } else {
            ESP_LOGW(CLOCK_TAG, "NTP returned invalid time");
        }
    } else {
        ESP_LOGW(CLOCK_TAG, "NTP sync failed, continuing with RTC time");
    }
}

/**
 * @brief Get the current date and time and update global variables
 */
static void get_date_time(void)
{
    time_t now;
    struct tm timeinfo;

    // Get current time based on available source
    if (time_source == TIME_SOURCE_RTC) {
        // Read time from RTC
        if (ds3231_read_time(&now) != ESP_OK) {
            ESP_LOGE(CLOCK_TAG, "Failed to read RTC time");
            time(&now); // Fallback to system time
        }
    } else {
        // Use system time (from NTP or internal)
        time(&now);
    }

    localtime_r(&now, &timeinfo);

    // Update our global time/date variables
    d_year = timeinfo.tm_year + 1900;
    d_month = timeinfo.tm_mon + 1; // tm_mon is 0-11
    d_day = timeinfo.tm_mday;
    t_hour = timeinfo.tm_hour;
    t_minute = timeinfo.tm_min;
    t_second = timeinfo.tm_sec;
    daysOfTheWeek_Val = timeinfo.tm_wday; // 0 = Sunday
}

/**
 * @brief Update time display on the screen
 */
static void update_time_display(void)
{
    // Get current date and time
    get_date_time();
    
    // Format the time and date strings
    char time_str[32];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", t_hour, t_minute, t_second);
    
    char date_str[64];
    snprintf(date_str, sizeof(date_str), "%s, %02d-%s-%d", 
             daysOfTheWeek[daysOfTheWeek_Val], d_day, namesOfMonths[d_month-1], d_year);

    // Update UI elements with time data
    if (lvgl_port_lock(-1)) {
        // Update all labels that show time
        lv_label_set_text(objects.label_time, time_str);
        lv_label_set_text(objects.label_time_1, time_str);
        lv_label_set_text(objects.label_date, date_str);
        lv_label_set_text(objects.label_date_1, date_str);
        
        // Update info labels with time source info
        char info_str[70];
        const char* source_text = "";
        switch (time_source) {
            case TIME_SOURCE_NTP: source_text = "NTP"; break;
            case TIME_SOURCE_RTC: source_text = "RTC"; break;
            default: source_text = "Internal"; break;
        }
        
        snprintf(info_str, sizeof(info_str), "Last data update: %s Source: %s", time_str, source_text);
        lv_label_set_text(objects.label_info, info_str);
        lv_label_set_text(objects.label_info_1, info_str);
        
        lvgl_port_unlock();
    }
}

/**
 * @brief Update weather data and UI with current temperature only
 */
static void update_weather_data(void)
{
    // Try to fetch data from OpenWeatherMap
    esp_err_t result = weather_client_update();
    if (result != ESP_OK) {
        ESP_LOGW(CLOCK_TAG, "Failed to fetch weather data: %s", esp_err_to_name(result));
        return; // Exit if we couldn't fetch data
    }
    
    ESP_LOGI(CLOCK_TAG, "Weather data updated successfully");

    // Update the UI with the current temperature
    if (lvgl_port_lock(-1)) {
        // Get current temperature
        float current_temp = weather_client_get_temperature();
        
        // Only update if we have a valid temperature
        if (!isnan(current_temp)) {
            char temp_str[16];
            snprintf(temp_str, sizeof(temp_str), "%.1f°C", current_temp);
            lv_label_set_text(objects.label_current_temperature, temp_str);
            lv_label_set_text(objects.label_current_temperature_1, temp_str);
            
            // Log the temperature update
            ESP_LOGI(CLOCK_TAG, "Updated temperature to: %s", temp_str);
        } else {
            ESP_LOGW(CLOCK_TAG, "Received invalid temperature value");
        }
        
        // Unlock the display
        lvgl_port_unlock();
    }
}


/**
 * @brief Timer callback to update time display
 */
static void time_update_timer_cb(lv_timer_t *timer)
{
    update_time_display();
}

/**
 * @brief Timer callback to update weather data
 */
static void weather_update_timer_cb(lv_timer_t *timer)
{
    update_weather_data();
}

/**
 * @brief Create UI for weather station
 */
static void create_weather_ui(void)
{
    // Initialize the EEZ Flow UI
    ui_init();
    
    // Initialize the weather client
    init_weather_client();
    
    // Create timer to update time display every second
    lv_timer_create(time_update_timer_cb, 1000, NULL);
    
    // Create timer to update weather data every 10 minutes (600000 ms)
    // For testing, we can use a shorter interval
    weather_update_timer = lv_timer_create(weather_update_timer_cb, 600000, NULL);
    
    // Perform initial time update, weather update will happen after WiFi connection
    update_time_display();
    // Weather data update will be triggered after WiFi connection
}

/**
 * @brief Main application entry point
 */
void app_main(void)
{
    // Suppress warning about unused TAG from waveshare_rgb_lcd_port.h
    SUPPRESS_UNUSED_WARNING(TAG);
    
    // Initialize the display first to ensure UI works regardless of time sync
    ESP_LOGI(CLOCK_TAG, "Initializing Waveshare ESP32-S3 RGB LCD");
    waveshare_esp32_s3_rgb_lcd_init();
    ESP_LOGI(CLOCK_TAG, "Turning on backlight");
    wavesahre_rgb_lcd_bl_on();
    ESP_LOGI(CLOCK_TAG, "Backlight turned on");
    
    // Default to internal time as fallback
    time_source = TIME_SOURCE_INTERNAL;
    
    // Set default initial time if RTC fails
    time_t default_time;
    struct tm default_timeinfo = {
        .tm_year = 2025 - 1900,
        .tm_mon = 5 - 1,
        .tm_mday = 18,
        .tm_hour = 10,
        .tm_min = 35,
        .tm_sec = 0
    };
    default_time = mktime(&default_timeinfo);
    
    // Variables for time handling
    time_t now;
    struct timeval tv;
    
    // First try to get time from DS3231 RTC
    ESP_LOGI(CLOCK_TAG, "Initializing DS3231 RTC");
    if (ds3231_init() == ESP_OK) {
        ESP_LOGI(CLOCK_TAG, "Reading time from DS3231 RTC");
        if (ds3231_read_time(&now) == ESP_OK) {
            tv.tv_sec = now;
            tv.tv_usec = 0;
            settimeofday(&tv, NULL);
            time_source = TIME_SOURCE_RTC;
            ESP_LOGI(CLOCK_TAG, "Time set from RTC");
        } else {
            ESP_LOGW(CLOCK_TAG, "Failed to read time from RTC, using default time");
            tv.tv_sec = default_time;
            tv.tv_usec = 0;
            settimeofday(&tv, NULL);
        }
    } else {
        ESP_LOGW(CLOCK_TAG, "Failed to initialize RTC, using default time");
        tv.tv_sec = default_time;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
    }
    
    // Create UI immediately to ensure it's displayed regardless of WiFi/NTP state
    ESP_LOGI(CLOCK_TAG, "Creating weather station UI");
    create_weather_ui();
    ESP_LOGI(CLOCK_TAG, "Weather station UI created");
    
    // Initial time display update
    update_time_display();
    
    // Start WiFi and NTP sync in background - won't block UI
    ESP_LOGI(CLOCK_TAG, "Starting WiFi/NTP sync in background");
    
    // Initialize NVS for WiFi storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize TCP/IP network interface (should be called only once in app)
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Create the event loop for WiFi and IP events (should be called only once)
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Initialize WiFi with proper network interface setup
    ESP_LOGI(CLOCK_TAG, "Starting WiFi initialization");
    
    // Register an IP event handler for when we get an IP address
    ESP_LOGI(CLOCK_TAG, "Registering IP event handler for NTP sync");
    
    static esp_event_handler_instance_t ip_event_handler;
    esp_err_t event_reg_result = esp_event_handler_instance_register(IP_EVENT, 
                                                                   IP_EVENT_STA_GOT_IP,
                                                                   &wifi_got_ip_event_handler,
                                                                   NULL,
                                                                   &ip_event_handler);
    if (event_reg_result != ESP_OK) {
        ESP_LOGW(CLOCK_TAG, "Failed to register IP event handler: %s", esp_err_to_name(event_reg_result));
    }
    
    // Start WiFi connection in the background
    // NTP sync will happen asynchronously via the IP event handler when we get an IP
    esp_err_t wifi_result = wifi_init_sta(wifi_ssid, wifi_password);
    if (wifi_result != ESP_OK) {
        ESP_LOGW(CLOCK_TAG, "WiFi initialization failed: %s. Using RTC time.", esp_err_to_name(wifi_result));
    } else {
        ESP_LOGI(CLOCK_TAG, "WiFi initialization started, waiting for connection in background");
    }
    
    ESP_LOGI(CLOCK_TAG, "App initialization complete, time source: %d", time_source);
}
