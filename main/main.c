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
 // #include "VL6180X.h" // Replaced with VL53L1X
#include "vl53l1x.h" // For VL53L1X sensor
#include "i2cdev.h"  // For I2C communication with VL53L1X via TeschRenan component      // For VL6180X sensor
 
 // Tag for logging
 static const char *MAIN_TAG = "Main";
 
 // Function to get the appropriate icon for a weather code
 static const void* get_icon_for_code(const char* code) {
     if (!code || strlen(code) < 3) {
         ESP_LOGW(MAIN_TAG, "Invalid weather icon code, using default");
         return &img_icon_01d_72p; // fallback to clear day
     }
     
     // Log the requested code
     ESP_LOGI(MAIN_TAG, "get_icon_for_code: Looking up icon for code: %s", code);
     
     // Define all icon mappings
     typedef struct {
         const char* code;
         const void* icon;
     } icon_mapping_t;
     
     static const icon_mapping_t icon_mappings[] = {
         {"01d", &img_icon_01d_72p},
         {"01n", &img_icon_01n_72p},
         {"02d", &img_icon_02d_72p},
         {"02n", &img_icon_02n_72p},
         {"03d", &img_icon_03d_03n_72p},
         {"03n", &img_icon_03d_03n_72p},
         {"04d", &img_icon_04d_04n_72p},
         {"04n", &img_icon_04d_04n_72p},
         {"09d", &img_icon_09d_09n_72p},
         {"09n", &img_icon_09d_09n_72p},
         {"10d", &img_icon_10d_72p},
         {"10n", &img_icon_10n_72p},
         {"11d", &img_icon_11d_11n_72p},
         {"11n", &img_icon_11d_11n_72p},
         {"13d", &img_icon_13d_13n_72p},
         {"13n", &img_icon_13d_13n_72p},
         {"50d", &img_icon_50d_50n_72p},
         {"50n", &img_icon_50d_50n_72p},
         {NULL, NULL}  // Terminator
     };
     
     // Find the matching icon
     for (int i = 0; icon_mappings[i].code != NULL; i++) {
         if (strcmp(code, icon_mappings[i].code) == 0) {
             ESP_LOGI(MAIN_TAG, "Found icon for %s at address %p", code, icon_mappings[i].icon);
             return icon_mappings[i].icon;
         }
     }
     
     ESP_LOGW(MAIN_TAG, "Unknown weather code: %s, using fallback icon", code);
     return &img_icon_01d_72p; // fallback to clear day
 }
 
 // Function to set weather icon on an LVGL image object
 static void set_weather_icon(const char* code, lv_obj_t* imgObj) {
     if (!imgObj) {
         ESP_LOGE(MAIN_TAG, "set_weather_icon: imgObj is NULL");
         return;
     }
     
     // Default to clear day if no valid code
     const char* icon_code = (code && strlen(code) >= 3) ? code : "01d";
     
     // Log the icon code being set
     ESP_LOGI(MAIN_TAG, "Setting weather icon: code='%s', imgObj=%p", icon_code, (void*)imgObj);
     
     // Get the icon data
     const void* icon = get_icon_for_code(icon_code);
     if (!icon) {
         ESP_LOGE(MAIN_TAG, "Failed to get icon for code: %s, using default", icon_code);
         icon = get_icon_for_code("01d");
     } else {
         ESP_LOGI(MAIN_TAG, "Got icon data: %p for code: %s", icon, icon_code);
     }
     
     // Set the new icon directly - no need to clear first with LVGL
     if (lvgl_port_lock(-1)) {
         ESP_LOGI(MAIN_TAG, "Setting image source for imgObj %p to icon %p", (void*)imgObj, icon);
         lv_img_set_src(imgObj, icon);
         lvgl_port_unlock();
         ESP_LOGI(MAIN_TAG, "Image source set successfully");
     } else {
         ESP_LOGE(MAIN_TAG, "Failed to acquire LVGL lock for icon update");
     }
     
     // Debug log for icon changes
     static const void* last_icon = NULL;
     if (icon != last_icon) {
         ESP_LOGI(MAIN_TAG, "Icon changed from %p to %p for code: %s", last_icon, icon, icon_code);
         last_icon = icon;
     }
 }
 
 // OpenWeatherMap configuration
 static const char *openWeatherMapApiKey = OPENWEATHER_API_KEY;
 static const char *lat = OPENWEATHER_LAT;
 static const char *lon = OPENWEATHER_LON;
 
 // VL53L1X Presence Sensor Configuration
 static VL53L1_Dev_t vl53l1x_dev;
 static I2cDrv i2c_bus_vl53l1x;

 // VL53L1X I2C Configuration (matches TeschRenan example structure)
 // Ensure these pins match your hardware connections for VL53L1X
 static const I2cDef I2CConfig_vl53l1x = {
     .i2cPort            = I2C_NUM_0, // Assuming same I2C port as VL6180X
     .i2cClockSpeed      = 400000,    // Standard I2C speed for VL53L1X
     .gpioSCLPin         = GPIO_NUM_9, // Your SCL pin
     .gpioSDAPin         = GPIO_NUM_8, // Your SDA pin
     .gpioPullup         = GPIO_PULLUP_ENABLE,
 };

 static const int PRESENCE_THRESHOLD_MM = 150; // Presence detected if distance < 150mm (can be adjusted for VL53L1X)
 static const int PRESENCE_TIMEOUT_S = 10;     // Turn off backlight after 10s of no presence
 static time_t last_presence_time = 0;
 static bool backlight_on = true;
 static bool presence_sensor_active = false; // To control sensor task activity

 #define WIFI_MAXIMUM_RETRY 5
#define NTP_SERVER "pool.ntp.org"
#define PRESENCE_TIMEOUT_SECONDS 10
#define PRESENCE_POLL_INTERVAL_MS 5000 // Poll every 5 seconds

 // Initialize weather client
 static void init_weather_client(void) {
     weather_client_init(openWeatherMapApiKey, lat, lon);
     ESP_LOGI(MAIN_TAG, "Weather client initialized with lat: %s, lon: %s", lat, lon);
 }
 
 // WiFi credentials
 const char *wifi_ssid = WIFI_SSID;
 const char *wifi_password = WIFI_PASSWORD;

 // WiFi event handler instance
 static esp_event_handler_instance_t ip_event_handler;

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
 static void update_hourly_forecast(void);
 static void update_daily_forecast(void); // Forward declaration
 
 // Presence Sensor functions
 static esp_err_t vl53l1x_sensor_init(void);
 static void presence_sensor_task(void *pvParameters);
 
 /**
  * @brief// IP address acquisition event handler for WiFi
  */
 static void wifi_got_ip_event_handler(void* arg, esp_event_base_t event_base,
                                          int32_t event_id, void* event_data)
 {
     ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
     ESP_LOGI(MAIN_TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
     
     // Now that we have an IP address, try to synchronize time with NTP
     ESP_LOGI(MAIN_TAG, "Starting NTP synchronization");
     
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
             ESP_LOGI(MAIN_TAG, "Time synchronized with NTP");
             
             // Update RTC with current time if available
             if (ds3231_init() == ESP_OK) {
                 ds3231_write_time(now);
                 ESP_LOGI(MAIN_TAG, "RTC updated with NTP time");
             }
             
             // Update display with new time
             update_time_display();
             
             // Now that we have WiFi and time is synchronized, update weather data
             ESP_LOGI(MAIN_TAG, "Triggering initial weather data update");
             update_weather_data();
             update_hourly_forecast();
             update_daily_forecast(); 
         } else {
             ESP_LOGW(MAIN_TAG, "NTP returned invalid time");
         }
     } else {
         ESP_LOGW(MAIN_TAG, "NTP sync failed, continuing with RTC time");
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
             ESP_LOGE(MAIN_TAG, "Failed to read RTC time");
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
         char info_str[100]; // Increased size for longer string
         const char* source_text = "";
         switch (time_source) {
             case TIME_SOURCE_NTP: source_text = "NTP"; break;
             case TIME_SOURCE_RTC: source_text = "RTC"; break;
             default: source_text = "Internal"; break;
         }
         
         // Get the last weather API update time
         time_t last_weather_update = weather_client_get_last_update_time();
         char weather_time_str[32] = "Never"; // Default if no update yet
         if (last_weather_update > 0) {
             struct tm timeinfo_weather;
             localtime_r(&last_weather_update, &timeinfo_weather);
             // Format: HH:MM DD-Mon (e.g., 14:30 24-May)
             strftime(weather_time_str, sizeof(weather_time_str), "%H:%M", &timeinfo_weather);
         }
         
         snprintf(info_str, sizeof(info_str), "Last data update: %s | Clock source: %s", weather_time_str, source_text);
         lv_label_set_text(objects.label_info, info_str);
         lv_label_set_text(objects.label_info_1, info_str);
         
         lvgl_port_unlock();
     }
 }
 
 /**
  * @brief Update the hourly forecast display
  */
  static void update_hourly_forecast(void) {
    ESP_LOGI(MAIN_TAG, "Starting hourly forecast update");
    
    // Get the number of available hourly forecasts
    uint8_t forecast_count = weather_client_get_hourly_forecast_count();
    if (forecast_count == 0) {
        ESP_LOGW(MAIN_TAG, "No hourly forecasts available");
        return;
    }
    
    ESP_LOGI(MAIN_TAG, "Found %d hourly forecasts", forecast_count);
    
    // Lock LVGL before updating the UI with retries
    #define LVGL_LOCK_MAX_RETRIES 5
    #define LVGL_LOCK_RETRY_DELAY_MS 20
    int lock_retries = 0;
    while (!lvgl_port_lock(0) && lock_retries < LVGL_LOCK_MAX_RETRIES) {
        ESP_LOGW(MAIN_TAG, "Failed to acquire LVGL lock for forecast update, retrying (%d/%d)...", 
                lock_retries + 1, LVGL_LOCK_MAX_RETRIES);
        vTaskDelay(pdMS_TO_TICKS(LVGL_LOCK_RETRY_DELAY_MS));
        lock_retries++;
    }
    
    if (lock_retries == LVGL_LOCK_MAX_RETRIES) {
        ESP_LOGE(MAIN_TAG, "Failed to acquire LVGL lock after %d retries, skipping forecast update", 
                LVGL_LOCK_MAX_RETRIES);
        return;
    }
    
    // Ensure the parent container is visible
    if (objects.view_1) {
        lv_obj_clear_flag(objects.view_1, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Update each hour's forecast
    for (int i = 0; i < 7 && i < forecast_count; i++) {
        hourly_forecast_t forecast;
        
        // Get the forecast for this hour (0-based index)
        if (!weather_client_get_hourly_forecast(i, &forecast)) {
            ESP_LOGW(MAIN_TAG, "No forecast available for hour %d", i + 1);
            continue;
        }
        
        // Ensure the icon code is valid and null-terminated
        char icon_code[8] = "01d";  // Default to clear day
        if (forecast.icon[0] != '\0') {
            strncpy(icon_code, forecast.icon, sizeof(icon_code) - 1);
            icon_code[sizeof(icon_code) - 1] = '\0';  // Ensure null termination
        }
        
        // Convert timestamp to local time
        struct tm timeinfo;
        time_t local_time = forecast.timestamp;
        localtime_r(&local_time, &timeinfo);
        
        // Update time label (format: HH:MM)
        char time_str[6];
        strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);
        
        // Update temperature (format: XX°)
        char temp_str[8] = "--°C";
        if (!isnan(forecast.temperature)) {
            snprintf(temp_str, sizeof(temp_str), "%.0f°C", forecast.temperature);
        }
        
        ESP_LOGI(MAIN_TAG, "Updating forecast %d: %s - %s - %s", 
                i, time_str, temp_str, icon_code);
        
        // Get the icon data
        const void* icon = get_icon_for_code(icon_code);
        if (!icon) {
            ESP_LOGW(MAIN_TAG, "No icon found for code: %s, using default", icon_code);
            icon = get_icon_for_code("01d");  // Fallback to clear day
        }
        
        // Update the appropriate UI elements based on the hour index
        lv_obj_t* label_time = NULL;
        lv_obj_t* label_temp = NULL;
        lv_obj_t* img_icon = NULL;
        
        switch (i) {
            case 0: 
                label_time = objects.label_1h;
                label_temp = objects.label_1h_temperature;
                img_icon = objects.image_1h_weather_icon;
                break;
            case 1:
                label_time = objects.label_2h;
                label_temp = objects.label_2h_temperature;
                img_icon = objects.image_2h_weather_icon;
                break;
            case 2:
                label_time = objects.label_3h;
                label_temp = objects.label_3h_temperature;
                img_icon = objects.image_3h_weather_icon;
                break;
            case 3:
                label_time = objects.label_4h;
                label_temp = objects.label_4h_temperature;
                img_icon = objects.image_4h_weather_icon;
                break;
            case 4:
                label_time = objects.label_5h;
                label_temp = objects.label_5h_temperature;
                img_icon = objects.image_5h_weather_icon;
                break;
            case 5:
                label_time = objects.label_6h;
                label_temp = objects.label_6h_temperature;
                img_icon = objects.image_6h_weather_icon;
                break;
            case 6:
                label_time = objects.label_7h;
                label_temp = objects.label_7h_temperature;
                img_icon = objects.image_7h_weather_icon;
                break;
            default:
                break;
        }
        
        // Update UI elements if they exist
        if (label_time) {
            lv_label_set_text(label_time, time_str);
            lv_obj_clear_flag(label_time, LV_OBJ_FLAG_HIDDEN);
        } else {
            ESP_LOGE(MAIN_TAG, "Label time for hour %d is NULL", i);
        }
        
        if (label_temp) {
            lv_label_set_text(label_temp, temp_str);
            lv_obj_clear_flag(label_temp, LV_OBJ_FLAG_HIDDEN);
        } else {
            ESP_LOGE(MAIN_TAG, "Label temp for hour %d is NULL", i);
        }
        
        if (img_icon) {
            lv_img_set_src(img_icon, icon);
            lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
            ESP_LOGI(MAIN_TAG, "Set icon for hour %d: %p", i, icon);
        } else {
            ESP_LOGE(MAIN_TAG, "Image icon for hour %d is NULL", i);
        }
        
        // Small delay to allow LVGL to process events
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGI(MAIN_TAG, "update_hourly_forecast: Loop finished. Attempting lv_refr_now.");
    // Force a screen refresh
    //lv_refr_now(NULL);
    ESP_LOGI(MAIN_TAG, "update_hourly_forecast: lv_refr_now completed. Attempting lvgl_port_unlock.");
    
    // Release the LVGL lock
    lvgl_port_unlock();
    ESP_LOGI(MAIN_TAG, "update_hourly_forecast: lvgl_port_unlock completed.");
    
    ESP_LOGI(MAIN_TAG, "Hourly forecast update complete");
}

/**
 * @brief Update weather data and refresh the UI
 */
static void update_weather_data(void)
{
    ESP_LOGI(MAIN_TAG, "Starting weather data update...");
    
    // Try to fetch data from OpenWeatherMap
    esp_err_t result = weather_client_update();
    if (result != ESP_OK) {
        ESP_LOGW(MAIN_TAG, "Failed to fetch weather data: %s", esp_err_to_name(result));
        return; // Exit if we couldn't fetch data
    }
    
    ESP_LOGI(MAIN_TAG, "Weather data updated successfully");

    // Get the complete weather data
    weather_data_t weather;
    if (!weather_client_get_data(&weather)) {
        ESP_LOGW(MAIN_TAG, "Failed to get weather data");
        return;
    }

    // Try to acquire LVGL lock with retries
    #define LOCK_RETRY_MS 50
    #define MAX_LOCK_RETRIES 5
    
    int retry_count = 0;
    while (!lvgl_port_lock(LOCK_RETRY_MS) && retry_count < MAX_LOCK_RETRIES) {
        ESP_LOGW(MAIN_TAG, "Failed to acquire LVGL lock for weather update, retry %d/%d", 
                retry_count + 1, MAX_LOCK_RETRIES);
        vTaskDelay(pdMS_TO_TICKS(10));
        retry_count++;
    }
    
    if (retry_count >= MAX_LOCK_RETRIES) {
        ESP_LOGE(MAIN_TAG, "Giving up on LVGL lock after %d retries, skipping weather update", MAX_LOCK_RETRIES);
        return;
    }
    
    // Update temperature if valid
    if (!isnan(weather.temperature)) {
        char temp_str[16];
        snprintf(temp_str, sizeof(temp_str), "%.1f°C", weather.temperature);
        
        // Update temperature labels
        lv_label_set_text(objects.label_current_temperature, temp_str);
        lv_label_set_text(objects.label_current_temperature_1, temp_str);
        
        ESP_LOGI(MAIN_TAG, "Updated temperature to: %s", temp_str);
    } else {
        ESP_LOGW(MAIN_TAG, "Received invalid temperature value");
    }

    // Update today's min/max temperature if valid
    // These are now sourced from daily[0] by weather_client.c
    if (!isnan(weather.temp_min) && !isnan(weather.temp_max)) {
        char min_temp_str[10]; // e.g., "Min: 10°"
        char max_temp_str[10]; // e.g., "Max: 20°"
        snprintf(min_temp_str, sizeof(min_temp_str), "%.0f°C", weather.temp_min);
        snprintf(max_temp_str, sizeof(max_temp_str), "%.0f°C", weather.temp_max);

        if (objects.label_current_temp_min) {
            lv_label_set_text(objects.label_current_temp_min, min_temp_str);
            lv_label_set_text(objects.label_current_temp_min_1, min_temp_str);
        }
        if (objects.label_current_temp_max) {
            lv_label_set_text(objects.label_current_temp_max, max_temp_str);
            lv_label_set_text(objects.label_current_temp_max_1, max_temp_str);
        }
        ESP_LOGI(MAIN_TAG, "Updated current day min/max temp to: %s / %s", min_temp_str, max_temp_str);
    } else {
        ESP_LOGW(MAIN_TAG, "Received invalid min/max temperature value for current day");
        // Optionally clear or set to default if objects exist
        if (objects.label_current_temp_min) {
            lv_label_set_text(objects.label_current_temp_min, "Min: --°");
        }
        if (objects.label_current_temp_max) {
            lv_label_set_text(objects.label_current_temp_max, "Max: --°");
        }
    }
    
    // Update weather description if available
    if (weather.description[0] != '\0') {
        // Capitalize first letter of description
        char desc[32];
        strlcpy(desc, weather.description, sizeof(desc));
        if (desc[0] >= 'a' && desc[0] <= 'z') {
            desc[0] = desc[0] - 'a' + 'A';
        }
        
        // Update description label
        lv_label_set_text(objects.label_weather_description, desc);
        ESP_LOGI(MAIN_TAG, "Weather: %s", desc);
    }
    
    // Update weather icon if available
    if (weather.icon[0] != '\0') {
        // Set the appropriate weather icon based on the condition
        set_weather_icon(weather.icon, objects.image_current_weather_icon);
        set_weather_icon(weather.icon, objects.image_current_weather_icon_1);
        ESP_LOGI(MAIN_TAG, "Weather icon: %s", weather.icon);
    }
    
    // Release the LVGL lock
    lvgl_port_unlock();
    
    // Update time display (it will handle its own locking)
    update_time_display();
    
    // Note: Don't call update_hourly_forecast() here as it will be called
    // by the timer. This prevents a potential infinite loop.
}

/**
 * @brief Timer callback for updating the time display
 * @param timer Timer handle (unused)
 */
static void time_update_timer_cb(lv_timer_t *timer)
{
    (void)timer; // Unused parameter
    update_time_display();
}

/**
 * @brief Timer callback for updating weather data and forecast
 * @param timer Timer handle (unused)
 */
static void weather_update_timer_cb(lv_timer_t *timer)
{
    (void)timer; // Unused parameter
    ESP_LOGI(MAIN_TAG, "Weather update timer triggered");
    
    // Update the main weather data
    ESP_LOGI(MAIN_TAG, "weather_update_timer_cb: Calling update_weather_data");
    update_weather_data();
    ESP_LOGI(MAIN_TAG, "weather_update_timer_cb: Returned from update_weather_data");
    
    // Update the hourly forecast
    ESP_LOGI(MAIN_TAG, "weather_update_timer_cb: Calling update_hourly_forecast");
    update_hourly_forecast();
    ESP_LOGI(MAIN_TAG, "weather_update_timer_cb: Returned from update_hourly_forecast");
    
    // Update the daily forecast
    ESP_LOGI(MAIN_TAG, "weather_update_timer_cb: Calling update_daily_forecast");
    update_daily_forecast(); 
    ESP_LOGI(MAIN_TAG, "weather_update_timer_cb: Returned from update_daily_forecast");
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
     // update_time_display(); // Commented out: This will be handled by deferred_ui_init_cb
     // Weather data update will be triggered after WiFi connection
 }

// Defer initial time display update to avoid LVGL dirty area modification during render
static void deferred_ui_init_cb(lv_timer_t *timer) {
    update_time_display();
    lv_timer_del(timer); // Run only once
}

/**
 * @brief Main application entry point
 */
 void app_main(void)
 {
     // Suppress warning about unused TAG from waveshare_rgb_lcd_port.h
     SUPPRESS_UNUSED_WARNING(TAG);
     
     // Initialize the display first to ensure UI works regardless of time sync
     ESP_LOGI(MAIN_TAG, "Initializing Waveshare ESP32-S3 RGB LCD");
     waveshare_esp32_s3_rgb_lcd_init();
     ESP_LOGI(MAIN_TAG, "Turning on backlight");
     wavesahre_rgb_lcd_bl_on();
     ESP_LOGI(MAIN_TAG, "Backlight turned on");
     
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
     ESP_LOGI(MAIN_TAG, "Initializing DS3231 RTC");
     if (ds3231_init() == ESP_OK) {
         ESP_LOGI(MAIN_TAG, "Reading time from DS3231 RTC");
         if (ds3231_read_time(&now) == ESP_OK) {
             tv.tv_sec = now;
             tv.tv_usec = 0;
             settimeofday(&tv, NULL);
             time_source = TIME_SOURCE_RTC;
             ESP_LOGI(MAIN_TAG, "Time set from RTC");
         } else {
             ESP_LOGW(MAIN_TAG, "Failed to read time from RTC, using default time");
             tv.tv_sec = default_time;
             tv.tv_usec = 0;
             settimeofday(&tv, NULL);
         }
     } else {
         ESP_LOGW(MAIN_TAG, "Failed to initialize RTC, using default time");
         tv.tv_sec = default_time;
         tv.tv_usec = 0;
         settimeofday(&tv, NULL);
     }
     
     // Add a small delay to allow LVGL task to run and stabilize before UI creation
     ESP_LOGI(MAIN_TAG, "Delaying before UI creation...");
     vTaskDelay(pdMS_TO_TICKS(150)); // Increased delay to 150ms

     // Create UI immediately to ensure it's displayed regardless of WiFi/NTP state
     ESP_LOGI(MAIN_TAG, "Creating weather station UI");
     create_weather_ui();
     ESP_LOGI(MAIN_TAG, "Weather station UI created");

     // Initialize VL53L1X sensor
     ESP_LOGI(MAIN_TAG, "Initializing VL53L1X sensor...");
     if (vl53l1x_sensor_init() == ESP_OK) {
         presence_sensor_active = true;
         ESP_LOGI(MAIN_TAG, "VL53L1X sensor initialized successfully.");
     } else {
         ESP_LOGE(MAIN_TAG, "Failed to initialize VL53L1X sensor.");
         presence_sensor_active = false;
     }

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
     ESP_LOGI(MAIN_TAG, "Starting WiFi initialization");
     
     // Register an IP event handler for when we get an IP address
     ESP_LOGI(MAIN_TAG, "Registering IP event handler for NTP sync");
     esp_err_t event_reg_result = esp_event_handler_instance_register(IP_EVENT, 
                                                                    IP_EVENT_STA_GOT_IP,
                                                                    &wifi_got_ip_event_handler,
                                                                    NULL,
                                                                    &ip_event_handler);
     if (event_reg_result != ESP_OK) {
         ESP_LOGW(MAIN_TAG, "Failed to register IP event handler: %s", esp_err_to_name(event_reg_result));
     }
     
     // Start WiFi connection in the background
     // NTP sync will happen asynchronously via the IP event handler when we get an IP
     esp_err_t wifi_result = wifi_init_sta(wifi_ssid, wifi_password);
     if (wifi_result != ESP_OK) {
         ESP_LOGW(MAIN_TAG, "WiFi initialization failed: %s. Using RTC time.", esp_err_to_name(wifi_result));
     } else {
         ESP_LOGI(MAIN_TAG, "WiFi initialization started, waiting for connection in background");
     }
     
     ESP_LOGI(MAIN_TAG, "App initialization complete, time source: %d", time_source);
         // Main loop to keep UI responsive
    xTaskCreate(presence_sensor_task, "presence_sensor_task", 4096, NULL, 5, NULL);
    while (1) {
        ui_tick();
        vTaskDelay(pdMS_TO_TICKS(10)); // 10 ms delay
    }
 }

// @brief Update the daily forecast display
static void update_daily_forecast(void)
{
    ESP_LOGI(MAIN_TAG, "ENTERED update_daily_forecast function"); // New very first log

    uint8_t total_daily_forecasts = weather_client_get_daily_forecast_count();
    ESP_LOGI(MAIN_TAG, "Total daily forecasts available (incl. today): %d", total_daily_forecasts);

    int days_to_display_in_panel = 0;
    if (total_daily_forecasts > 1) {
        days_to_display_in_panel = total_daily_forecasts - 1;
    }
    if (days_to_display_in_panel > 7) {
        days_to_display_in_panel = 7;
    }
    ESP_LOGI(MAIN_TAG, "Calculated days_to_display_in_panel: %d", days_to_display_in_panel);

    // Acquire LVGL lock ONCE before updating all daily forecast UI elements
    ESP_LOGI(MAIN_TAG, "Attempting to acquire LVGL lock for entire daily forecast update...");
    if (!lvgl_port_lock(-1)) {
        ESP_LOGE(MAIN_TAG, "FAILED to acquire LVGL lock for daily forecast update. Aborting.");
        return; // Cannot proceed without the lock
    }
    ESP_LOGI(MAIN_TAG, "LVGL lock acquired for daily forecast update.");

    for (int i = 0; i < 7; ++i) {
        ESP_LOGI(MAIN_TAG, "Daily forecast loop: Start iteration i = %d", i);

        lv_obj_t* label_day = NULL;
        lv_obj_t* label_temp_min = NULL;
        lv_obj_t* label_temp_max = NULL;
        lv_obj_t* img_icon = NULL;

        switch (i) {
            case 0: label_day = objects.label_1d; label_temp_min = objects.label_1d_temp_min; label_temp_max = objects.label_1d_temp_max; img_icon = objects.image_1d_weather_icon; break;
            case 1: label_day = objects.label_2d; label_temp_min = objects.label_2d_temp_min; label_temp_max = objects.label_2d_temp_max; img_icon = objects.image_2d_weather_icon; break;
            case 2: label_day = objects.label_3d; label_temp_min = objects.label_3d_temp_min; label_temp_max = objects.label_3d_temp_max; img_icon = objects.image_3d_weather_icon; break;
            case 3: label_day = objects.label_4d; label_temp_min = objects.label_4d_temp_min; label_temp_max = objects.label_4d_temp_max; img_icon = objects.image_4d_weather_icon; break;
            case 4: label_day = objects.label_5d; label_temp_min = objects.label_5d_temp_min; label_temp_max = objects.label_5d_temp_max; img_icon = objects.image_5d_weather_icon; break;
            case 5: label_day = objects.label_6d; label_temp_min = objects.label_6d_temp_min; label_temp_max = objects.label_6d_temp_max; img_icon = objects.image_6d_weather_icon; break;
            case 6: label_day = objects.label_7d; label_temp_min = objects.label_7d_temp_min; label_temp_max = objects.label_7d_temp_max; img_icon = objects.image_7d_weather_icon; break;
        }
        ESP_LOGD(MAIN_TAG, "Daily forecast loop i = %d: UI elements assigned", i);

        if (i < days_to_display_in_panel) {
            daily_forecast_t forecast_data; // Renamed to avoid conflict if 'forecast' is a global or static
            ESP_LOGD(MAIN_TAG, "Daily forecast loop i = %d: Getting forecast for API daily[%d]", i, i + 1);
            if (weather_client_get_daily_forecast(i + 1, &forecast_data)) {
                char day_name_str[4];
                char temp_min_str[8];
                char temp_max_str[8];

                struct tm timeinfo_daily;
                localtime_r(&forecast_data.timestamp, &timeinfo_daily);
                strftime(day_name_str, sizeof(day_name_str), "%a", &timeinfo_daily);
                snprintf(temp_min_str, sizeof(temp_min_str), "%.0f°C", forecast_data.temp_min);
                snprintf(temp_max_str, sizeof(temp_max_str), "%.0f°C", forecast_data.temp_max);

                ESP_LOGI(MAIN_TAG, "Panel day %d (API daily[%d]): %s - Min: %s, Max: %s - Icon: %s. Attempting LVGL lock.",
                         i, i + 1, day_name_str, temp_min_str, temp_max_str, forecast_data.icon);

                // LVGL lock is already held, proceed with UI updates
                if (label_day) lv_label_set_text(label_day, day_name_str);
                if (label_temp_min) lv_label_set_text(label_temp_min, temp_min_str);
                if (label_temp_max) lv_label_set_text(label_temp_max, temp_max_str);
                if (img_icon) set_weather_icon(forecast_data.icon, img_icon);

            } else {
                ESP_LOGW(MAIN_TAG, "Failed to get daily forecast for panel day %d (API daily[%d]). Clearing UI.", i, i + 1);
                // LVGL lock is already held, proceed with UI updates
                if (label_day) lv_label_set_text(label_day, "---");
                if (label_temp_min) lv_label_set_text(label_temp_min, "--°C");
                if (label_temp_max) lv_label_set_text(label_temp_max, "--°C");
                if (img_icon) lv_img_set_src(img_icon, NULL); // Clear icon
            }
        } else { // Hide unused panel days
            ESP_LOGD(MAIN_TAG, "Daily forecast loop i = %d: Hiding unused panel day.", i);
            // LVGL lock is already held, proceed with UI updates
            if (label_day) lv_obj_add_flag(label_day, LV_OBJ_FLAG_HIDDEN);
            if (label_temp_min) lv_obj_add_flag(label_temp_min, LV_OBJ_FLAG_HIDDEN);
            if (label_temp_max) lv_obj_add_flag(label_temp_max, LV_OBJ_FLAG_HIDDEN);
            if (img_icon) lv_obj_add_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
        }
        // Remove any vTaskDelay from inside this loop if present
    }

    // Release the LVGL lock ONCE after all UI updates are done
    lvgl_port_unlock();
    ESP_LOGI(MAIN_TAG, "LVGL lock released after daily forecast update.");

    ESP_LOGI(MAIN_TAG, "COMPLETED update_daily_forecast function");
}

// Initialize VL53L1X sensor
static esp_err_t vl53l1x_sensor_init(void) {
    ESP_LOGI(MAIN_TAG, "Initializing VL53L1X sensor...");

    // Setup I2C bus configuration for VL53L1X
    i2c_bus_vl53l1x.def = &I2CConfig_vl53l1x;

    // Initialize the sensor using the TeschRenan/VL53L1X component's init function
    // The component's vl53l1xInit function takes VL53L1_Dev_t* and I2cDrv*
    // It internally handles i2cdev_init and sets the device address.
    if (!vl53l1xInit(&vl53l1x_dev, &i2c_bus_vl53l1x)) {
        ESP_LOGE(MAIN_TAG, "Failed to initialize VL53L1X sensor (vl53l1xInit failed).");
        presence_sensor_active = false;
        return ESP_FAIL;
    }
    ESP_LOGI(MAIN_TAG, "VL53L1X sensor vl53l1xInit successful. Device I2C address: 0x%02X", vl53l1x_dev.I2cDevAddr);

    VL53L1_Error status;

    // Wait for device to boot up
    status = VL53L1_WaitDeviceBooted(&vl53l1x_dev);
    if (status != VL53L1_ERROR_NONE) {
        ESP_LOGE(MAIN_TAG, "VL53L1_WaitDeviceBooted failed: %d", status);
        presence_sensor_active = false;
        return ESP_FAIL;
    }
    ESP_LOGI(MAIN_TAG, "VL53L1X device booted.");

    // Initialize sensor data structures
    status = VL53L1_DataInit(&vl53l1x_dev);
    if (status != VL53L1_ERROR_NONE) {
        ESP_LOGE(MAIN_TAG, "VL53L1_DataInit failed: %d", status);
        presence_sensor_active = false;
        return ESP_FAIL;
    }
    ESP_LOGI(MAIN_TAG, "VL53L1X DataInit successful.");

    // Perform static initialization
    status = VL53L1_StaticInit(&vl53l1x_dev);
    if (status != VL53L1_ERROR_NONE) {
        ESP_LOGE(MAIN_TAG, "VL53L1_StaticInit failed: %d", status);
        presence_sensor_active = false;
        return ESP_FAIL;
    }
    ESP_LOGI(MAIN_TAG, "VL53L1X StaticInit successful.");

    // Configure sensor for long distance mode
    status = VL53L1_SetDistanceMode(&vl53l1x_dev, VL53L1_DISTANCEMODE_LONG);
    if (status != VL53L1_ERROR_NONE) {
        ESP_LOGE(MAIN_TAG, "VL53L1_SetDistanceMode (LONG) failed: %d", status);
        presence_sensor_active = false;
        return ESP_FAIL; // If mode setting fails, it's a critical error
    } else {
        ESP_LOGI(MAIN_TAG, "VL53L1X distance mode set to LONG.");
    }

    // Set measurement timing budget (e.g., 50ms). Longer budget for better accuracy at longer distances.
    // Min for LONG mode is 33ms. Max is not strictly defined but affects update rate.
    // 50000 microseconds = 50ms.
    uint32_t timing_budget_us = 50000;
    status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(&vl53l1x_dev, timing_budget_us);
    if (status != VL53L1_ERROR_NONE) {
        ESP_LOGE(MAIN_TAG, "VL53L1_SetMeasurementTimingBudgetMicroSeconds (%lu us) failed: %d", timing_budget_us, status);
        presence_sensor_active = false;
        return ESP_FAIL; // Critical if timing budget cannot be set
    } else {
        ESP_LOGI(MAIN_TAG, "VL53L1X timing budget set to %lu us.", timing_budget_us);
    }

    // The presence_sensor_task will handle starting/stopping measurements for each read.
    // So, no need to call VL53L1_StartMeasurement here after initial setup.

    presence_sensor_active = true;
    ESP_LOGI(MAIN_TAG, "VL53L1X sensor initialized successfully and is active.");
    return ESP_OK;
}



// Presence Sensor Task
// Presence Sensor Task
static void presence_sensor_task(void *pvParameters) {
    uint16_t range_mm = 0; // Stores the measured range
    time_t current_time;
    VL53L1_RangingMeasurementData_t ranging_data;
    VL53L1_Error status;

    ESP_LOGI(MAIN_TAG, "Presence sensor task started (VL53L1X).");

    // Initialize last_presence_time to current time to keep backlight on initially
    time(&last_presence_time);

    int consecutive_failures = 0;
    const int MAX_SENSOR_FAILURES = 10; // Renamed for clarity
    const uint32_t DATA_READY_TIMEOUT_MS = 200; // Timeout for waiting for data ready (e.g. 4x timing budget)

    while (1) {
        if (!presence_sensor_active) {
            static int inactive_count = 0;
            inactive_count++;
            if (inactive_count % 10 == 0) {
                ESP_LOGW(MAIN_TAG, "presence_sensor_task (VL53L1X) running but presence_sensor_active is false (count=%d)", inactive_count);
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // Check periodically if sensor becomes active
            continue;
        }

        ESP_LOGD(MAIN_TAG, "VL53L1X: Starting measurement cycle.");
        status = VL53L1_StartMeasurement(&vl53l1x_dev);
        if (status != VL53L1_ERROR_NONE) {
            ESP_LOGE(MAIN_TAG, "VL53L1_StartMeasurement failed: %d. Incrementing failures.", status);
            consecutive_failures++;
        } else {
            uint8_t data_ready = 0;
            uint32_t start_time_ms = esp_log_timestamp(); // For timeout

            // Poll for data ready with timeout
            while (data_ready == 0 && (esp_log_timestamp() - start_time_ms < DATA_READY_TIMEOUT_MS)) {
                status = VL53L1_GetMeasurementDataReady(&vl53l1x_dev, &data_ready);
                if (status != VL53L1_ERROR_NONE) {
                    ESP_LOGE(MAIN_TAG, "VL53L1_GetMeasurementDataReady failed: %d. Breaking poll loop.", status);
                    break; // Exit polling loop on error
                }
                if (data_ready == 0) {
                    vTaskDelay(pdMS_TO_TICKS(10)); // Wait briefly before polling again
                }
            }

            if (data_ready && status == VL53L1_ERROR_NONE) {
                status = VL53L1_GetRangingMeasurementData(&vl53l1x_dev, &ranging_data);
                if (status == VL53L1_ERROR_NONE) {
                    if (ranging_data.RangeStatus == VL53L1_RANGESTATUS_RANGE_VALID) {
                        range_mm = ranging_data.RangeMilliMeter;
                        ESP_LOGI(MAIN_TAG, "VL53L1X measured distance: %d mm (Status: %d)", range_mm, ranging_data.RangeStatus);
                        consecutive_failures = 0; // Reset failures on successful valid read

                        if (range_mm < PRESENCE_THRESHOLD_MM) {
                            time(&last_presence_time); // Update last seen time
                            if (!backlight_on) {
                                ESP_LOGI(MAIN_TAG, "Presence detected (VL53L1X), turning backlight ON.");
                                wavesahre_rgb_lcd_bl_on();
                                backlight_on = true;
                            }
                        }
                    } else {
                        // Valid measurement but range status indicates an issue (e.g., out of bounds, signal fail)
                        ESP_LOGW(MAIN_TAG, "VL53L1X measurement valid but range status not OK: %d. Distance: %d mm. Incrementing failures.", ranging_data.RangeStatus, ranging_data.RangeMilliMeter);
                        consecutive_failures++;
                    }
                } else {
                    ESP_LOGE(MAIN_TAG, "VL53L1_GetRangingMeasurementData failed: %d. Incrementing failures.", status);
                    consecutive_failures++;
                }
            } else if (status == VL53L1_ERROR_NONE && data_ready == 0) {
                ESP_LOGW(MAIN_TAG, "VL53L1X data not ready within timeout. Incrementing failures.");
                consecutive_failures++;
            } else {
                 // Error occurred in VL53L1_GetMeasurementDataReady or VL53L1_StartMeasurement
                 ESP_LOGE(MAIN_TAG, "VL53L1X read cycle failed before GetRangingMeasurementData. API status: %d. Incrementing failures.", status);
                 consecutive_failures++; // Already incremented if StartMeasurement failed
            }
            
            // Clear interrupt and stop measurement. The next loop iteration will start a new one.
            // This is a simple way to handle single-shot style measurements per loop iteration.
            // VL53L1_ClearInterruptAndStartMeasurement could be used if an interrupt pin was configured and used.
            VL53L1_StopMeasurement(&vl53l1x_dev); // Stop to ensure clean state for next StartMeasurement
        }

        if (consecutive_failures >= MAX_SENSOR_FAILURES) {
            ESP_LOGW(MAIN_TAG, "%d consecutive VL53L1X read failures. Will continue attempting reads.", consecutive_failures);
            consecutive_failures = 0; // Reset counter to allow re-logging if failures persist
            if (!backlight_on) {
                ESP_LOGI(MAIN_TAG, "Ensuring backlight is ON after multiple VL53L1X sensor read failures.");
                wavesahre_rgb_lcd_bl_on();
                backlight_on = true;
            }
        }

        time(&current_time);
        if (backlight_on && (difftime(current_time, last_presence_time) > PRESENCE_TIMEOUT_S)) {
            ESP_LOGI(MAIN_TAG, "No presence detected for %d seconds (VL53L1X), turning backlight OFF.", PRESENCE_TIMEOUT_S);
            wavesahre_rgb_lcd_bl_off();
            backlight_on = false;
        }

        vTaskDelay(pdMS_TO_TICKS(PRESENCE_POLL_INTERVAL_MS));
    }
}