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
        char temp_str[8] = "--°";
        if (!isnan(forecast.temperature)) {
            snprintf(temp_str, sizeof(temp_str), "%.0f°", forecast.temperature);
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
    
    // Force a screen refresh
    lv_refr_now(NULL);
    
    // Release the LVGL lock
    lvgl_port_unlock();
    
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
    update_weather_data();
    
    // Update the hourly forecast
    ESP_LOGI(MAIN_TAG, "Updating hourly forecast from timer");
    update_hourly_forecast();
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
     
     // Create UI immediately to ensure it's displayed regardless of WiFi/NTP state
     ESP_LOGI(MAIN_TAG, "Creating weather station UI");
     create_weather_ui();
     ESP_LOGI(MAIN_TAG, "Weather station UI created");
     
// Defer initial time display update to avoid LVGL dirty area modification during render
void deferred_ui_init_cb(lv_timer_t *timer) {
    update_time_display();
    lv_timer_del(timer);
}

// ... other code ...

    lv_timer_create(deferred_ui_init_cb, 100, NULL); // 100 ms after UI creation
     
     // Start WiFi and NTP sync in background - won't block UI
     ESP_LOGI(MAIN_TAG, "Starting WiFi/NTP sync in background");
     
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
    while (1) {
        ui_tick();
        vTaskDelay(pdMS_TO_TICKS(10)); // 10 ms delay
    }
 }
 