#include "UI/actions.h"
#include "UI/screens.h"
#include "UI/ui.h"
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
        const char * obj_name = lv_obj_get_user_data(target);
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
    
    // Check if we have appropriate global variables defined in the UI
    // These would be set by EEZ Studio UI
    #ifdef FLOW_GLOBAL_VARIABLE_G_ENTITY
    auto entity_var = flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_G_ENTITY);
    if (entity_var.isString()) {
        entity = entity_var.getString();
        ESP_LOGI(TAG, "Entity: %s", entity);
    }
    #endif
    
    #ifdef FLOW_GLOBAL_VARIABLE_G_PAYLOAD
    auto payload_var = flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_G_PAYLOAD);
    if (payload_var.isString()) {
        payload = payload_var.getString();
        ESP_LOGI(TAG, "Payload: %s", payload);
    }
    #endif
    
    // If we have both entity and payload, make the Home Assistant API call
    if (entity && payload) {
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
        // If we don't have entity and payload, just handle screen navigation
        ESP_LOGI(TAG, "No entity/payload variables found, performing screen navigation");
        
        // Navigation between screens based on EnumsScreen values
        lv_obj_t *current_screen = lv_scr_act();
        
        // Check if we're on the main screen
        if (current_screen == objects.main) {
            ESP_LOGI(TAG, "Currently on main screen, navigating to PC screen");
            lv_scr_load_anim(objects.pc, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
        } 
        // Check if we're on the PC screen
        else if (current_screen == objects.pc) {
            ESP_LOGI(TAG, "Currently on PC screen, returning to main screen");
            lv_scr_load_anim(objects.main, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
        } 
        // Fallback to main screen if we're on an unknown screen
        else {
            ESP_LOGI(TAG, "On unknown screen, returning to main screen");
            lv_scr_load(objects.main);
        }
        
        ESP_LOGI(TAG, "Screen navigation performed");
    }
    
    ESP_LOGI(TAG, "--- Finished Home Assistant REST API Call ---");
}
