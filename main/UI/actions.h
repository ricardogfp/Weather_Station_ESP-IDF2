#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif


enum {
    ACTION_REST_API_PROPERTY_G_ENTITY,
    ACTION_REST_API_PROPERTY_G_PAYLOAD,
};
extern void action_rest_api(lv_event_t * e);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/