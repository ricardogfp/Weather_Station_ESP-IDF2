#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;

static void event_handler_cb_main_main(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_main_obj0(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 63, 0, e);
    }
}

static void event_handler_cb_main_obj1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 65, 0, e);
    }
}

static void event_handler_cb_pc_pc(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_pc_obj2(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            assignBooleanProperty(flowState, 3, 3, value, "Failed to assign Checked state");
        }
    }
    
    lv_obj_t *ta = lv_event_get_target(e);
    if (event == LV_EVENT_VALUE_CHANGED && lv_obj_has_state(ta, LV_STATE_CHECKED)) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 0, e);
    }
    if (event == LV_EVENT_VALUE_CHANGED && !lv_obj_has_state(ta, LV_STATE_CHECKED)) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 1, e);
    }
}

static void event_handler_cb_pc_obj3(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 15, 0, e);
    }
}

static void event_handler_cb_pc_obj4(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 17, 0, e);
    }
}

static void event_handler_cb_pc_view_1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 21, 0, e);
    }
}

static void event_handler_cb_pc_view_1_1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 23, 0, e);
    }
}

static void event_handler_cb_pc_view_1_2(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 25, 0, e);
    }
}

static void event_handler_cb_pc_view_1_3(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 27, 0, e);
    }
}

static void event_handler_cb_pc_view_1_4(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 29, 0, e);
    }
}

static void event_handler_cb_pc_view_1_5(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 31, 0, e);
    }
}

static void event_handler_cb_pc_view_1_6(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 33, 0, e);
    }
}

static void event_handler_cb_pc_view_1_7(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 35, 0, e);
    }
}

static void event_handler_cb_pc_view_1_8(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 37, 0, e);
    }
}

static void event_handler_cb_pc_view_1_9(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 39, 0, e);
    }
}

void create_screen_main() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_main_main, LV_EVENT_ALL, flowState);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff464653), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xff212121), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 784, 95);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff262635), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // label_city
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_city = obj;
            lv_obj_set_pos(obj, 112, 62);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "City");
        }
        {
            // label_time
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_time = obj;
            lv_obj_set_pos(obj, 300, 23);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_letter_space(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00:00:00");
        }
        {
            // label_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_date = obj;
            lv_obj_set_pos(obj, 319, 72);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Wednesday, 00-Dec0-2025");
        }
        {
            // image_Current_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_current_weather_icon = obj;
            lv_obj_set_pos(obj, 19, 18);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_01d_72p);
        }
        {
            // label_weather_description
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_weather_description = obj;
            lv_obj_set_pos(obj, 111, 28);
            lv_obj_set_size(obj, 159, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Clear sky");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 8, 111);
            lv_obj_set_size(obj, 784, 310);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff262635), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // label_current_temperature
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_current_temperature = obj;
            lv_obj_set_pos(obj, 636, 18);
            lv_obj_set_size(obj, 113, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_current_temp_min
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_current_temp_min = obj;
            lv_obj_set_pos(obj, 616, 59);
            lv_obj_set_size(obj, 73, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 689, 59);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "/");
        }
        {
            // label_current_temp_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_current_temp_max = obj;
            lv_obj_set_pos(obj, 696, 59);
            lv_obj_set_size(obj, 73, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_1h_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_1h_weather_icon = obj;
            lv_obj_set_pos(obj, 28, 140);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_01d_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_1h
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_1h = obj;
            lv_obj_set_pos(obj, 28, 123);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "10:00 PM");
        }
        {
            // label_1h_temperature
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_1h_temperature = obj;
            lv_obj_set_pos(obj, 28, 214);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_2h_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_2h_weather_icon = obj;
            lv_obj_set_pos(obj, 143, 140);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_01n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_2h
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_2h = obj;
            lv_obj_set_pos(obj, 143, 123);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "11:00 PM");
        }
        {
            // label_2h_temperature
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_2h_temperature = obj;
            lv_obj_set_pos(obj, 143, 214);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_3h_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_3h_weather_icon = obj;
            lv_obj_set_pos(obj, 258, 141);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_03d_03n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_3h
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_3h = obj;
            lv_obj_set_pos(obj, 258, 123);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "12:00 PM");
        }
        {
            // label_3h_temperature
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_3h_temperature = obj;
            lv_obj_set_pos(obj, 258, 214);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_4h_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_4h_weather_icon = obj;
            lv_obj_set_pos(obj, 371, 140);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_04d_04n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_4h
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_4h = obj;
            lv_obj_set_pos(obj, 371, 123);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "01:00 AM");
        }
        {
            // label_4h_temperature
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_4h_temperature = obj;
            lv_obj_set_pos(obj, 371, 214);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_5h_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_5h_weather_icon = obj;
            lv_obj_set_pos(obj, 480, 140);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_50d_50n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_5h
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_5h = obj;
            lv_obj_set_pos(obj, 480, 123);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "02:00 AM");
        }
        {
            // label_5h_temperature
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_5h_temperature = obj;
            lv_obj_set_pos(obj, 480, 213);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_6h_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_6h_weather_icon = obj;
            lv_obj_set_pos(obj, 594, 141);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_50d_50n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_6h
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_6h = obj;
            lv_obj_set_pos(obj, 594, 123);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "03:00 AM");
        }
        {
            // label_6h_temperature
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_6h_temperature = obj;
            lv_obj_set_pos(obj, 594, 214);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_1d_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_1d_weather_icon = obj;
            lv_obj_set_pos(obj, 28, 292);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_13d_13n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_1d
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_1d = obj;
            lv_obj_set_pos(obj, 28, 275);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Sat");
        }
        {
            // label_1d_temp_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_1d_temp_max = obj;
            lv_obj_set_pos(obj, 28, 370);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_2d_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_2d_weather_icon = obj;
            lv_obj_set_pos(obj, 143, 293);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_50d_50n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_2d_temp_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_2d_temp_max = obj;
            lv_obj_set_pos(obj, 143, 371);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_3d_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_3d_weather_icon = obj;
            lv_obj_set_pos(obj, 258, 294);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_50d_50n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_3d_temp_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_3d_temp_max = obj;
            lv_obj_set_pos(obj, 258, 371);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_4d_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_4d_weather_icon = obj;
            lv_obj_set_pos(obj, 371, 293);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_13d_13n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_4d_temp_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_4d_temp_max = obj;
            lv_obj_set_pos(obj, 371, 371);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_5d_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_5d_weather_icon = obj;
            lv_obj_set_pos(obj, 480, 293);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_13d_13n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_5d_temp_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_5d_temp_max = obj;
            lv_obj_set_pos(obj, 480, 371);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_6d_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_6d_weather_icon = obj;
            lv_obj_set_pos(obj, 594, 293);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_13d_13n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_6d_temp_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_6d_temp_max = obj;
            lv_obj_set_pos(obj, 594, 371);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_2d
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_2d = obj;
            lv_obj_set_pos(obj, 143, 276);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Sun");
        }
        {
            // label_3d
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_3d = obj;
            lv_obj_set_pos(obj, 258, 276);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Mon");
        }
        {
            // label_4d
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_4d = obj;
            lv_obj_set_pos(obj, 371, 275);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Tue");
        }
        {
            // label_5d
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_5d = obj;
            lv_obj_set_pos(obj, 480, 275);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Wed");
        }
        {
            // label_6d
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_6d = obj;
            lv_obj_set_pos(obj, 594, 275);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Thu");
        }
        {
            // label_1d_temp_min
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_1d_temp_min = obj;
            lv_obj_set_pos(obj, 28, 393);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_2d_temp_min
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_2d_temp_min = obj;
            lv_obj_set_pos(obj, 143, 394);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_3d_temp_min
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_3d_temp_min = obj;
            lv_obj_set_pos(obj, 258, 394);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_4d_temp_min
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_4d_temp_min = obj;
            lv_obj_set_pos(obj, 371, 393);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_5d_temp_min
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_5d_temp_min = obj;
            lv_obj_set_pos(obj, 480, 393);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_6d_temp_min
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_6d_temp_min = obj;
            lv_obj_set_pos(obj, 594, 393);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_7h_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_7h_weather_icon = obj;
            lv_obj_set_pos(obj, 705, 142);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_13d_13n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_7h
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_7h = obj;
            lv_obj_set_pos(obj, 705, 123);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "04:00 AM");
        }
        {
            // label_7h_temperature
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_7h_temperature = obj;
            lv_obj_set_pos(obj, 705, 214);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // image_7d_weather_icon
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_7d_weather_icon = obj;
            lv_obj_set_pos(obj, 705, 294);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_11d_11n_72p);
            lv_img_set_zoom(obj, 200);
        }
        {
            // label_7d_temp_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_7d_temp_max = obj;
            lv_obj_set_pos(obj, 705, 370);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_7d
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_7d = obj;
            lv_obj_set_pos(obj, 705, 276);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Fri");
        }
        {
            // label_7d_temp_min
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_7d_temp_min = obj;
            lv_obj_set_pos(obj, 705, 394);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 10, 431);
            lv_obj_set_size(obj, 784, 40);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff262635), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // label_info
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_info = obj;
            lv_obj_set_pos(obj, 207, 444);
            lv_obj_set_size(obj, 386, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Connecting to WiFi");
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 512, 36);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_add_event_cb(obj, event_handler_cb_main_obj0, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "I");
                }
            }
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 570, 36);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_add_event_cb(obj, event_handler_cb_main_obj1, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "O");
                }
            }
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
}

void create_screen_pc() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.pc = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_pc_pc, LV_EVENT_ALL, flowState);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff464653), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 8, 111);
            lv_obj_set_size(obj, 784, 310);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff262635), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 784, 95);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff262635), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 234, 29);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_obj2, LV_EVENT_ALL, flowState);
        }
        {
            // label_city_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_city_1 = obj;
            lv_obj_set_pos(obj, 112, 62);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "City");
        }
        {
            // label_time_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_time_1 = obj;
            lv_obj_set_pos(obj, 300, 23);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_letter_space(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00:00:00");
        }
        {
            // label_date_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_date_1 = obj;
            lv_obj_set_pos(obj, 319, 72);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Wednesday, 00-Dec0-2025");
        }
        {
            // image_Current_weather_icon_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_current_weather_icon_1 = obj;
            lv_obj_set_pos(obj, 19, 18);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_01d_72p);
        }
        {
            // label_weather_description_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_weather_description_1 = obj;
            lv_obj_set_pos(obj, 111, 28);
            lv_obj_set_size(obj, 159, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Clear sky");
        }
        {
            // label_current_temperature_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_current_temperature_1 = obj;
            lv_obj_set_pos(obj, 636, 18);
            lv_obj_set_size(obj, 113, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            // label_current_temp_min_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_current_temp_min_1 = obj;
            lv_obj_set_pos(obj, 616, 59);
            lv_obj_set_size(obj, 73, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 689, 59);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "/");
        }
        {
            // label_current_temp_max_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_current_temp_max_1 = obj;
            lv_obj_set_pos(obj, 696, 59);
            lv_obj_set_size(obj, 73, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-00.0°C");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 10, 431);
            lv_obj_set_size(obj, 784, 40);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff262635), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 419, 141);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_cockpit);
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 512, 36);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_obj3, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "I");
                }
            }
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 570, 36);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_obj4, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "O");
                }
            }
        }
        {
            // label_info_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_info_1 = obj;
            lv_obj_set_pos(obj, 207, 444);
            lv_obj_set_size(obj, 386, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfff0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Connecting to WiFi");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 88, 124);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_plane);
        }
        {
            // View 1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1 = obj;
            lv_obj_set_pos(obj, 474, 214);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj13 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 1");
                }
            }
        }
        {
            // View 1_1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_1 = obj;
            lv_obj_set_pos(obj, 547, 204);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_1, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj14 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 2");
                }
            }
        }
        {
            // View 1_2
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_2 = obj;
            lv_obj_set_pos(obj, 400, 246);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_2, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj15 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 3");
                }
            }
        }
        {
            // View 1_3
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_3 = obj;
            lv_obj_set_pos(obj, 80, 166);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_3, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj16 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 4");
                }
            }
        }
        {
            // View 1_4
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_4 = obj;
            lv_obj_set_pos(obj, 547, 126);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_4, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj17 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 5");
                }
            }
        }
        {
            // View 1_5
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_5 = obj;
            lv_obj_set_pos(obj, 259, 166);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_5, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj18 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 6");
                }
            }
        }
        {
            // View 1_6
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_6 = obj;
            lv_obj_set_pos(obj, 80, 316);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_6, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj19 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 7");
                }
            }
        }
        {
            // View 1_7
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_7 = obj;
            lv_obj_set_pos(obj, 80, 240);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_7, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj20 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 8");
                }
            }
        }
        {
            // View 1_8
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_8 = obj;
            lv_obj_set_pos(obj, 259, 316);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_8, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj21 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 9");
                }
            }
        }
        {
            // View 1_9
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.view_1_9 = obj;
            lv_obj_set_pos(obj, 259, 240);
            lv_obj_set_size(obj, 63, 63);
            lv_obj_add_event_cb(obj, event_handler_cb_pc_view_1_9, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffd0d3d6), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj22 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3989ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "View 0");
                }
            }
        }
    }
    
    tick_screen_pc();
}

void tick_screen_pc() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
    {
        bool new_val = evalBooleanProperty(flowState, 3, 3, "Failed to evaluate Checked state");
        bool cur_val = lv_obj_has_state(objects.obj2, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.obj2;
            if (new_val) lv_obj_add_state(objects.obj2, LV_STATE_CHECKED);
            else lv_obj_clear_state(objects.obj2, LV_STATE_CHECKED);
            tick_value_change_obj = NULL;
        }
    }
}


static const char *screen_names[] = { "Main", "PC" };
static const char *object_names[] = { "main", "pc", "obj0", "obj1", "obj2", "obj3", "obj4", "view_1", "view_1_1", "view_1_2", "view_1_3", "view_1_4", "view_1_5", "view_1_6", "view_1_7", "view_1_8", "view_1_9", "obj5", "label_city", "label_time", "label_date", "image_current_weather_icon", "label_weather_description", "obj6", "label_current_temperature", "label_current_temp_min", "obj7", "label_current_temp_max", "image_1h_weather_icon", "label_1h", "label_1h_temperature", "image_2h_weather_icon", "label_2h", "label_2h_temperature", "image_3h_weather_icon", "label_3h", "label_3h_temperature", "image_4h_weather_icon", "label_4h", "label_4h_temperature", "image_5h_weather_icon", "label_5h", "label_5h_temperature", "image_6h_weather_icon", "label_6h", "label_6h_temperature", "image_1d_weather_icon", "label_1d", "label_1d_temp_max", "image_2d_weather_icon", "label_2d_temp_max", "image_3d_weather_icon", "label_3d_temp_max", "image_4d_weather_icon", "label_4d_temp_max", "image_5d_weather_icon", "label_5d_temp_max", "image_6d_weather_icon", "label_6d_temp_max", "label_2d", "label_3d", "label_4d", "label_5d", "label_6d", "label_1d_temp_min", "label_2d_temp_min", "label_3d_temp_min", "label_4d_temp_min", "label_5d_temp_min", "label_6d_temp_min", "image_7h_weather_icon", "label_7h", "label_7h_temperature", "image_7d_weather_icon", "label_7d_temp_max", "label_7d", "label_7d_temp_min", "obj8", "label_info", "obj9", "obj10", "label_city_1", "label_time_1", "label_date_1", "image_current_weather_icon_1", "label_weather_description_1", "label_current_temperature_1", "label_current_temp_min_1", "obj11", "label_current_temp_max_1", "obj12", "label_info_1", "obj13", "obj14", "obj15", "obj16", "obj17", "obj18", "obj19", "obj20", "obj21", "obj22" };


typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_pc,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    eez_flow_init_screen_names(screen_names, sizeof(screen_names) / sizeof(const char *));
    eez_flow_init_object_names(object_names, sizeof(object_names) / sizeof(const char *));
    
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
    create_screen_pc();
}
