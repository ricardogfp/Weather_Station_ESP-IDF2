#include "home_assistant.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "esp_wifi.h"
#include <string.h>

static const char *TAG = "HOME_ASSISTANT";

// Initialize Home Assistant client
esp_err_t home_assistant_init(void) {
    ESP_LOGI(TAG, "Initializing Home Assistant client");
    return ESP_OK;
}

// Send a command to a Home Assistant entity
esp_err_t home_assistant_update_entity(const char* entity, const char* payload) {
    ESP_LOGI(TAG, "=== home_assistant_update_entity called ===");
    ESP_LOGI(TAG, "Entity: %s", entity ? entity : "NULL");
    ESP_LOGI(TAG, "Payload: %s", payload ? payload : "NULL");
    
    if (!entity || !payload) {
        ESP_LOGE(TAG, "Invalid entity or payload (null)");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(entity) == 0 || strlen(payload) == 0) {
        ESP_LOGE(TAG, "Empty entity or payload");
        return ESP_ERR_INVALID_ARG;
    }

    // Check Wi-Fi connection
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi not connected: %s (0x%x)", esp_err_to_name(ret), ret);
        return ESP_ERR_WIFI_NOT_CONNECT;
    }
    ESP_LOGI(TAG, "WiFi connected to SSID: %s, RSSI: %d", ap_info.ssid, ap_info.rssi);

    // Determine service based on payload
    char service[32];
    if (strcmp(payload, "on") == 0) {
        strcpy(service, "light/turn_on");
    } else if (strcmp(payload, "off") == 0) {
        strcpy(service, "light/turn_off");
    } else if (strcmp(payload, "pressed") == 0) {
        strcpy(service, "button/press");
    } else {
        ESP_LOGE(TAG, "Unsupported payload value: %s", payload);
        return ESP_ERR_INVALID_ARG;
    }

    // Prepare URL
    char url[256];
    snprintf(url, sizeof(url), "%s/api/services/%s", HOME_ASSISTANT_URL, service);
    ESP_LOGI(TAG, "Connecting to URL: %s", url);

    // Prepare headers
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", HOME_ASSISTANT_TOKEN);
    ESP_LOGI(TAG, "Authorization header: %s", auth_header);

    // Prepare JSON payload
    char json_payload[256];
    snprintf(json_payload, sizeof(json_payload), "{\"entity_id\": \"%s\"}", entity);
    ESP_LOGI(TAG, "JSON payload: %s", json_payload);

    // Initialize HTTP client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .skip_cert_common_name_check = true,
    };
    ESP_LOGI(TAG, "Initializing HTTP client with URL: %s", url);
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "HTTP client initialized successfully");

    // Set headers
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", auth_header);

    // Set POST data
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));

    // Perform the request
    ESP_LOGI(TAG, "Sending HTTP POST request to %s", url);
    ESP_LOGI(TAG, "Request headers:");
    ESP_LOGI(TAG, "  Content-Type: application/json");
    ESP_LOGI(TAG, "  Authorization: %s", auth_header);
    ESP_LOGI(TAG, "Request body: %s", json_payload);
    
    esp_err_t err;
    err = esp_http_client_perform(client);
    
    ESP_LOGI(TAG, "HTTP request completed with status: %s (0x%x)", esp_err_to_name(err), err);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP Response Status = %d", status_code);
        ESP_LOGI(TAG, "Content-Length = %lld", (long long)esp_http_client_get_content_length(client));
        
        // Read response
        char response_buffer[256] = {0};
        int content_length = esp_http_client_get_content_length(client);
        if (content_length <= sizeof(response_buffer)) {
            int read_len = esp_http_client_read(client, response_buffer, content_length);
            if (read_len > 0) {
                ESP_LOGI(TAG, "Response body: %s", response_buffer);
            }
        }
        
        // Success is 200 or 201
        if (status_code == 200 || status_code == 201) {
            esp_http_client_cleanup(client);
            return ESP_OK;
        } else {
            ESP_LOGE(TAG, "HTTP error, status code: %d", status_code);
            esp_http_client_cleanup(client);
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
    ESP_LOGI(TAG, "=== home_assistant_update_entity completed with status: %s (0x%x) ===\n", esp_err_to_name(err), err);
    return err;
    }
}
