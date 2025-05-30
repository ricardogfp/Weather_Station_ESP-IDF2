#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_MQTT_TOPIC = 0,
    FLOW_GLOBAL_VARIABLE_CONN = 1,
    FLOW_GLOBAL_VARIABLE_LIGHTSTATE = 2,
    FLOW_GLOBAL_VARIABLE_G_ENTITY = 3,
    FLOW_GLOBAL_VARIABLE_G_PAYLOAD = 4
};

// Native global variables



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/