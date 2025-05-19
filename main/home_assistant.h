#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Home Assistant configuration
#define HOME_ASSISTANT_URL "http://192.168.6.11:8123"
#define HOME_ASSISTANT_TOKEN "asdasdaso"  // Replace with your actual token

/**
 * @brief Initialize Home Assistant client
 * 
 * @return ESP_OK if successful, otherwise an error code
 */
esp_err_t home_assistant_init(void);

/**
 * @brief Send a command to a Home Assistant entity
 * 
 * @param entity The entity ID to update (e.g., "light.living_room")
 * @param payload The command to send (e.g., "on", "off", "pressed")
 * @return ESP_OK if successful, otherwise an error code
 */
esp_err_t home_assistant_update_entity(const char* entity, const char* payload);

#ifdef __cplusplus
}
#endif
