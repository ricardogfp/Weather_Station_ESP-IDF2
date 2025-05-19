#include "wifi_manager.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Include SNTP headers
#include "lwip/apps/sntp.h"

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
    ESP_LOGI(TAG, "Initializing SNTP");
    
    // Configure timezone to local time
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    
    // Initialize SNTP with multiple server options
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "time.google.com");
    sntp_setservername(2, "time.cloudflare.com");
    
    // SNTP sync interval is set by default
    
    // Actually start the SNTP service
    sntp_init();
    ESP_LOGI(TAG, "SNTP initialized, waiting for time sync");
    
    // Wait for time to be set with proper timeout
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15; // Increased retry count for better chance of success
    bool time_synced = false;
    
    // Wait for time sync with timeout
    while (retry < retry_count && !time_synced) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry + 1, retry_count);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Print current time status
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%c", &timeinfo);
        ESP_LOGI(TAG, "Current time: %s", time_str);
        
        // Check if time has been set properly
        if (timeinfo.tm_year >= (2020 - 1900)) {
            ESP_LOGI(TAG, "Time is synchronized!");
            time_synced = true;
            break;
        }
        retry++;
    }
    
    if (!time_synced) {
        ESP_LOGW(TAG, "Failed to get time from NTP server");
        sntp_stop(); // Stop SNTP client on failure
        return ESP_FAIL;
    }
    
    time(&now);
    localtime_r(&now, &timeinfo);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%c", &timeinfo);
    ESP_LOGI(TAG, "Synchronized time: %s", time_str);
    
    if (timeinfo.tm_year < (2020 - 1900)) {
        ESP_LOGW(TAG, "Time not synchronized yet");
        return ESP_FAIL;
    }
    
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Current time: %s", strftime_buf);
    
    return ESP_OK;
}
