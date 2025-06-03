#include "presence_sensor.h"
#include "waveshare_rgb_lcd_port.h" // For wavesahre_rgb_lcd_bl_on/off
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "secrets.h"

#define PRESENCE_TIMEOUT_S 10  // Or move to Kconfig
#define PRESENCE_POLL_INTERVAL_MS 1000
#define LD2410C_PRESENCE_GPIO 6

static const char *PRESENCE_TAG = "PRESENCE";
static time_t last_presence_time = 0;
static bool backlight_on = true;

static void presence_sensor_task(void *pvParameters)
{
    while (1) {
        int presence = gpio_get_level(LD2410C_PRESENCE_GPIO);
        time_t now;
        time(&now);

        if (presence) {
            ESP_LOGI(PRESENCE_TAG, "Human presence detected!");
            last_presence_time = now;
            if (!backlight_on) {
                wavesahre_rgb_lcd_bl_on();
                backlight_on = true;
                ESP_LOGI(PRESENCE_TAG, "Backlight turned ON due to presence.");
            }
        } else {
            ESP_LOGI(PRESENCE_TAG, "No presence detected.");
            if (backlight_on && difftime(now, last_presence_time) > PRESENCE_TIMEOUT_S) {
                wavesahre_rgb_lcd_bl_off();
                backlight_on = false;
                ESP_LOGI(PRESENCE_TAG, "Backlight turned OFF after absence timeout.");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(PRESENCE_POLL_INTERVAL_MS));
    }
}

void presence_sensor_init(void)
{
    // GPIO config could go here if not done elsewhere
    xTaskCreate(presence_sensor_task, "presence_sensor_task", 4096, NULL, 2, NULL);
}
