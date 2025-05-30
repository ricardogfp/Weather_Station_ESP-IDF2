idf_component_register(
    SRCS "ui_actions.cpp" "weather_client.c" "waveshare_rgb_lcd_port.c" "main.c" "lvgl_port.c" "ds3231.c" "wifi_manager.c" 
        
         "UI/ui.c" "UI/eez-flow.cpp" "UI/screens.c"
         "UI/images.c" "UI/styles.c" "home_assistant.c"
         "UI/images/ui_image_icon_01d_72p.c" "UI/images/ui_image_icon_01n_72p.c"
         "UI/images/ui_image_icon_02d_72p.c" "UI/images/ui_image_icon_02n_72p.c"
         "UI/images/ui_image_icon_03d_03n_72p.c" "UI/images/ui_image_icon_04d_04n_72p.c"
         "UI/images/ui_image_icon_09d_09n_72p.c" "UI/images/ui_image_icon_10d_72p.c"
         "UI/images/ui_image_icon_10n_72p.c" "UI/images/ui_image_icon_11d_11n_72p.c"
         "UI/images/ui_image_icon_13d_13n_72p.c" "UI/images/ui_image_icon_50d_50n_72p.c"
         "UI/images/ui_image_cockpit.c" "UI/images/ui_image_plane.c"
    INCLUDE_DIRS "." "UI"
    REQUIRES esp_http_client json nvs_flash esp_event esp_wifi esp_netif esp-tls VL6180X driver)

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
