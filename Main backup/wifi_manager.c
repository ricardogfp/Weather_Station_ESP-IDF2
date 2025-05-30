#include "wifi_manager.h"
#include <string.h>
#include <time.h>
#include <sys/param.h>
#include <inttypes.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "secrets.h"

// SNTP configuration
#define SNTP_SERVER_COUNT 3
#define SNTP_SYNC_INTERVAL_MS (3600 * 1000)  // 1 hour in ms

// Forward declaration of the time sync callback
static void time_sync_notification_cb(struct timeval *tv);

// Time sync notification callback
static void time_sync_notification_cb(struct timeval *tv)
{
    if (!tv) return;
    
    time_t now = tv->tv_sec;
    struct tm timeinfo;
    char strftime_buf[64];
    
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    // Get current timezone information
    const char* tz = getenv("TZ");
    const char* tz_info = tz ? tz : "UTC";
    
    ESP_LOGI("SNTP", "Time synchronized: %s.%06ld %s (TZ: %s)", 
             strftime_buf, 
             (long)tv->tv_usec,
             timeinfo.tm_isdst ? "CEST" : "CET",
             tz_info);
}

#define EXAMPLE_ESP_MAXIMUM_RETRY 5

static const char *TAG = "wifi_manager";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi station started, connecting...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "WiFi connected, waiting for IP...");
        // Don't set the connected bit yet - wait for IP
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP (%d/%d)", s_retry_num, EXAMPLE_ESP_MAXIMUM_RETRY);
        } else {
            ESP_LOGW(TAG, "Connect to the AP failed after %d attempts", EXAMPLE_ESP_MAXIMUM_RETRY);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_init_sta(const char *ssid, const char *password)
{
    // Initialize return value
    esp_err_t ret = ESP_OK;
    
    // Initialize event group for WiFi events
    if (s_wifi_event_group == NULL) {
        s_wifi_event_group = xEventGroupCreate();
        if (s_wifi_event_group == NULL) {
            ESP_LOGE(TAG, "Failed to create WiFi event group");
            return ESP_FAIL;
        }
    }
    
    // Create default WiFi STA network interface
    ESP_LOGI(TAG, "Creating default WiFi STA network interface");
    esp_netif_t *wifi_sta = esp_netif_create_default_wifi_sta();
    if (wifi_sta == NULL) {
        ESP_LOGE(TAG, "Failed to create default WiFi STA network interface");
        return ESP_FAIL;
    }
    
    // Initialize WiFi
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&wifi_init_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Register WiFi event handler
    ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WiFi event handler: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Register IP event handler
    ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register IP event handler: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configure WiFi station
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    // Copy SSID and password to WiFi configuration
    if (ssid) {
        strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    }
    
    if (password) {
        strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }
    
    // Set WiFi mode to station mode
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Set WiFi configuration
    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi configuration: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Start WiFi
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WiFi: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Connecting to WiFi...");
    ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to WiFi: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Wait for connection - with multiple retries
    for (int retry = 0; retry < 3; retry++) {
        ESP_LOGI(TAG, "Waiting for WiFi connection (attempt %d/3)...", retry + 1);
        
        // Wait for connection with timeout
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                              WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                              pdFALSE,
                                              pdFALSE,
                                              5000 / portTICK_PERIOD_MS);
        
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "Connected to WiFi successfully");
            return ESP_OK;
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGW(TAG, "Failed to connect to WiFi");
            if (retry < 2) {
                ESP_LOGI(TAG, "Retrying connection...");
                esp_wifi_disconnect();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                esp_wifi_connect();
            }
        } else {
            ESP_LOGW(TAG, "WiFi connection timeout");
            if (retry < 2) {
                ESP_LOGI(TAG, "Retrying connection...");
                esp_wifi_disconnect();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                esp_wifi_connect();
            }
        }
    }
    
    // Clean up WiFi on failure
    esp_wifi_stop();
    ESP_LOGW(TAG, "Failed to connect to WiFi after multiple attempts");
    return ESP_FAIL;
}

// Simple time sync function without callback

esp_err_t sntp_init_sync(void)
{
    ESP_LOGI("SNTP", "Initializing SNTP service");
    
    // Stop any existing SNTP service
    if (esp_sntp_enabled()) {
        esp_sntp_stop();
    }
    
    // Set timezone to Central European Time (CET/CEST)
    const char* tz = "CET-1CEST,M3.5.0,M10.5.0/3";
    setenv("TZ", tz, 1);
    tzset();
    ESP_LOGI("SNTP", "Initializing SNTP with timezone: %s", tz);

    // Set SNTP operating mode
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    // Set NTP servers
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.nist.gov");
    esp_sntp_setservername(2, "time.google.com");
    // Set sync interval (ms)
    esp_sntp_set_sync_interval(SNTP_SYNC_INTERVAL_MS);
    // Set sync notification callback
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    // Start SNTP service
    esp_sntp_init();
    ESP_LOGI("SNTP", "SNTP service started");
    
    // Log initial time status
    time_t now = 0;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
    ESP_LOGI("SNTP", "Current time: %s (timestamp: %" PRId64 ")", time_str, (int64_t)now);
    
    ESP_LOGI("SNTP", "SNTP service started, waiting for time sync");
    
    // Wait for time to be set with proper timeout
    int retry = 0;
    const int retry_count = 15;  // 15 seconds total timeout
    const int retry_delay_ms = 1000;  // 1 second between retries
    bool time_synced = false;
    
    ESP_LOGI("SNTP", "Waiting for time synchronization...");
    
    while (retry < retry_count) {
        if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
            time_synced = true;
            break;
        }
        
        // Log current time status
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
        ESP_LOGI("SNTP", "Waiting for sync... (%d/%d) Current: %s", 
                 retry + 1, retry_count, time_str);
        
        vTaskDelay(pdMS_TO_TICKS(retry_delay_ms));
        retry++;
    }
    
    if (!time_synced) {
        ESP_LOGE("SNTP", "Failed to synchronize time after %d seconds", retry_count);
        esp_sntp_stop();
        return ESP_ERR_TIMEOUT;
    }
    
    // Final time check
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
    
    if (timeinfo.tm_year < (2020 - 1900)) {
        ESP_LOGW("SNTP", "Time appears to be invalid (year: %d)", timeinfo.tm_year + 1900);
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI("SNTP", "Time successfully synchronized: %s", time_str);
    return ESP_OK;
}
