#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_icon_01d_72p;
extern const lv_img_dsc_t img_icon_01n_72p;
extern const lv_img_dsc_t img_icon_02d_72p;
extern const lv_img_dsc_t img_icon_02n_72p;
extern const lv_img_dsc_t img_icon_03d_03n_72p;
extern const lv_img_dsc_t img_icon_04d_04n_72p;
extern const lv_img_dsc_t img_icon_09d_09n_72p;
extern const lv_img_dsc_t img_icon_10d_72p;
extern const lv_img_dsc_t img_icon_10n_72p;
extern const lv_img_dsc_t img_icon_11d_11n_72p;
extern const lv_img_dsc_t img_icon_13d_13n_72p;
extern const lv_img_dsc_t img_icon_50d_50n_72p;
extern const lv_img_dsc_t img_plane;
extern const lv_img_dsc_t img_cockpit;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[14];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/