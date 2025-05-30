#include "UI/actions.h"
#include "UI/screens.h"
#include "UI/ui.h"
#include "UI/vars.h"
#include "UI/eez-flow.h"
#include "esp_log.h"
#include "home_assistant.h"


static const char* TAG = "UI_ACTIONS";

// Reference to UI objects from screens.h
extern objects_t objects;

// Implementation of action_rest_api used for UI navigation and Home Assistant integration
void action_rest_api(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    
    ESP_LOGI(TAG, "--- Event Received ---");
    ESP_LOGI(TAG, "Event code: 0x%x", code);
    
    // Log common event types
    switch(code) {
        case LV_EVENT_PRESSED: ESP_LOGI(TAG, "Event: PRESSED"); break;
        case LV_EVENT_PRESSING: ESP_LOGI(TAG, "Event: PRESSING"); break;
        case LV_EVENT_PRESS_LOST: ESP_LOGI(TAG, "Event: PRESS_LOST"); break;
        case LV_EVENT_SHORT_CLICKED: ESP_LOGI(TAG, "Event: SHORT_CLICKED"); break;
        case LV_EVENT_CLICKED: ESP_LOGI(TAG, "Event: CLICKED"); break;
        case LV_EVENT_LONG_PRESSED: ESP_LOGI(TAG, "Event: LONG_PRESSED"); break;
        case LV_EVENT_LONG_PRESSED_REPEAT: ESP_LOGI(TAG, "Event: LONG_PRESSED_REPEAT"); break;
        case LV_EVENT_RELEASED: ESP_LOGI(TAG, "Event: RELEASED"); break;
        default: ESP_LOGI(TAG, "Event: UNKNOWN (0x%x)", code);
    }
    
    if (target) {
        const char * obj_name = (const char *)lv_obj_get_user_data(target); // Added explicit cast for C++
        ESP_LOGI(TAG, "Target object: %p, Name: %s", target, obj_name ? obj_name : "unnamed");
    } else {
        ESP_LOGI(TAG, "No target object");
    }
    
    // Process button events (including RELEASED)
    if (code != LV_EVENT_CLICKED && 
        code != LV_EVENT_SHORT_CLICKED && 
        code != LV_EVENT_PRESSED &&
        code != LV_EVENT_RELEASED) {
        ESP_LOGI(TAG, "Not a click/press/release event, ignoring");
        return;
    }
    
    ESP_LOGI(TAG, "Processing button event: %s (0x%x)", 
             code == LV_EVENT_CLICKED ? "CLICKED" :
             code == LV_EVENT_SHORT_CLICKED ? "SHORT_CLICKED" :
             code == LV_EVENT_PRESSED ? "PRESSED" : "RELEASED",
             code);
    
    // Try to get global variables for entity and payload if they exist
    const char* entity = NULL;
    const char* payload = NULL;
    
    ESP_LOGI(TAG, "Attempting to retrieve global variables G_ENTITY and G_PAYLOAD.");

    // Check if we have appropriate global variables defined in the UI
    // These would be set by EEZ Studio UI
    ESP_LOGI(TAG, "Looking for G_ENTITY (%d)", FLOW_GLOBAL_VARIABLE_G_ENTITY);
    auto entity_var = eez::flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_G_ENTITY);
    ESP_LOGI(TAG, "G_ENTITY raw Value type: %d (IsString: %s)", entity_var.getType(), entity_var.isString() ? "true" : "false");
    if (entity_var.isString()) {
        entity = entity_var.getString();
        ESP_LOGI(TAG, "G_ENTITY retrieved as string: '%s'", entity ? entity : "NULL_STR");
    } else {
        // Log even if not a string, to see what getString might return or if it's null
        const char* temp_entity_str = entity_var.getString();
        ESP_LOGW(TAG, "G_ENTITY is not a string. getString() returns: '%s'", temp_entity_str ? temp_entity_str : "NULL_STR");
    }
    
    ESP_LOGI(TAG, "Looking for G_PAYLOAD (%d)", FLOW_GLOBAL_VARIABLE_G_PAYLOAD);
    auto payload_var = eez::flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_G_PAYLOAD);
    ESP_LOGI(TAG, "G_PAYLOAD raw Value type: %d (IsString: %s)", payload_var.getType(), payload_var.isString() ? "true" : "false");
    if (payload_var.isString()) {
        payload = payload_var.getString();
        ESP_LOGI(TAG, "G_PAYLOAD retrieved as string: '%s'", payload ? payload : "NULL_STR");
    } else {
        const char* temp_payload_str = payload_var.getString();
        ESP_LOGW(TAG, "G_PAYLOAD is not a string. getString() returns: '%s'", temp_payload_str ? temp_payload_str : "NULL_STR");
    }
    
    // If we have both entity and payload, make the Home Assistant API call
    if (entity && payload && strlen(entity) > 0 && strlen(payload) > 0) { 
        ESP_LOGI(TAG, "--- Making Home Assistant REST API Call ---");
        ESP_LOGI(TAG, "Entity: %s", entity);
        ESP_LOGI(TAG, "Payload: %s", payload);
        esp_err_t result = home_assistant_update_entity(entity, payload);
        ESP_LOGI(TAG, "Home Assistant API call result: %s (0x%x)", 
                esp_err_to_name(result), result);
        
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "REST API call successful");
        } else {
            ESP_LOGE(TAG, "REST API call failed with error: %s", esp_err_to_name(result));
        }
    } else {
        // If we don't have entity and payload, just log it. Screen navigation removed.
        ESP_LOGW(TAG, "Entity ('%s') or Payload ('%s') are missing or empty. No Home Assistant call will be made. No screen navigation.", 
                 entity ? entity : "NULL_OR_UNDEFINED", 
                 payload ? payload : "NULL_OR_UNDEFINED");
    }
    
    ESP_LOGI(TAG, "--- Finished processing action_rest_api ---"); 
}
