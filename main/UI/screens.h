#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *pc;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *view_1;
    lv_obj_t *view_1_1;
    lv_obj_t *view_1_2;
    lv_obj_t *view_1_3;
    lv_obj_t *view_1_4;
    lv_obj_t *view_1_5;
    lv_obj_t *view_1_6;
    lv_obj_t *view_1_7;
    lv_obj_t *view_1_8;
    lv_obj_t *view_1_9;
    lv_obj_t *obj5;
    lv_obj_t *label_city;
    lv_obj_t *label_time;
    lv_obj_t *label_date;
    lv_obj_t *image_current_weather_icon;
    lv_obj_t *label_weather_description;
    lv_obj_t *obj6;
    lv_obj_t *label_current_temperature;
    lv_obj_t *label_current_temp_min;
    lv_obj_t *obj7;
    lv_obj_t *label_current_temp_max;
    lv_obj_t *image_1h_weather_icon;
    lv_obj_t *label_1h;
    lv_obj_t *label_1h_temperature;
    lv_obj_t *image_2h_weather_icon;
    lv_obj_t *label_2h;
    lv_obj_t *label_2h_temperature;
    lv_obj_t *image_3h_weather_icon;
    lv_obj_t *label_3h;
    lv_obj_t *label_3h_temperature;
    lv_obj_t *image_4h_weather_icon;
    lv_obj_t *label_4h;
    lv_obj_t *label_4h_temperature;
    lv_obj_t *image_5h_weather_icon;
    lv_obj_t *label_5h;
    lv_obj_t *label_5h_temperature;
    lv_obj_t *image_6h_weather_icon;
    lv_obj_t *label_6h;
    lv_obj_t *label_6h_temperature;
    lv_obj_t *image_1d_weather_icon;
    lv_obj_t *label_1d;
    lv_obj_t *label_1d_temp_max;
    lv_obj_t *image_2d_weather_icon;
    lv_obj_t *label_2d_temp_max;
    lv_obj_t *image_3d_weather_icon;
    lv_obj_t *label_3d_temp_max;
    lv_obj_t *image_4d_weather_icon;
    lv_obj_t *label_4d_temp_max;
    lv_obj_t *image_5d_weather_icon;
    lv_obj_t *label_5d_temp_max;
    lv_obj_t *image_6d_weather_icon;
    lv_obj_t *label_6d_temp_max;
    lv_obj_t *label_2d;
    lv_obj_t *label_3d;
    lv_obj_t *label_4d;
    lv_obj_t *label_5d;
    lv_obj_t *label_6d;
    lv_obj_t *label_1d_temp_min;
    lv_obj_t *label_2d_temp_min;
    lv_obj_t *label_3d_temp_min;
    lv_obj_t *label_4d_temp_min;
    lv_obj_t *label_5d_temp_min;
    lv_obj_t *label_6d_temp_min;
    lv_obj_t *image_7h_weather_icon;
    lv_obj_t *label_7h;
    lv_obj_t *label_7h_temperature;
    lv_obj_t *image_7d_weather_icon;
    lv_obj_t *label_7d_temp_max;
    lv_obj_t *label_7d;
    lv_obj_t *label_7d_temp_min;
    lv_obj_t *obj8;
    lv_obj_t *label_info;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *label_city_1;
    lv_obj_t *label_time_1;
    lv_obj_t *label_date_1;
    lv_obj_t *image_current_weather_icon_1;
    lv_obj_t *label_weather_description_1;
    lv_obj_t *label_current_temperature_1;
    lv_obj_t *label_current_temp_min_1;
    lv_obj_t *obj11;
    lv_obj_t *label_current_temp_max_1;
    lv_obj_t *obj12;
    lv_obj_t *label_info_1;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_PC = 2,
};

void create_screen_main();
void tick_screen_main();

void create_screen_pc();
void tick_screen_pc();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/