#include "ds3231.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "waveshare_rgb_lcd_port.h"
#include <sys/time.h>

static const char *DS3231_TAG = "ds3231";

// Convert binary coded decimal to binary
uint8_t bcd2bin(uint8_t val)
{
    return val - 6 * (val >> 4);
}

// Convert binary to binary coded decimal
uint8_t bin2bcd(uint8_t val)
{
    return val + 6 * (val / 10);
}

esp_err_t ds3231_init(void)
{
    // I2C is already initialized in waveshare_esp32_s3_rgb_lcd_init()
    ESP_LOGI(DS3231_TAG, "DS3231 initialized");
    return ESP_OK;
}

esp_err_t ds3231_read_time(time_t *time)
{
    uint8_t data[7];
    uint8_t start_reg = DS3231_REG_SECONDS;
    esp_err_t ret;

    // Read 7 bytes starting from seconds register
    ret = i2c_master_write_read_device(I2C_MASTER_NUM, DS3231_I2C_ADDR, &start_reg, 1, 
                                       data, 7, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(DS3231_TAG, "Failed to read from DS3231: %s", esp_err_to_name(ret));
        return ret;
    }

    struct tm timeinfo = {
        .tm_sec = bcd2bin(data[0] & 0x7F),
        .tm_min = bcd2bin(data[1]),
        .tm_hour = bcd2bin(data[2] & 0x3F), // 24hr mode
        .tm_mday = bcd2bin(data[4]),
        .tm_mon = bcd2bin(data[5] & 0x1F) - 1, // 0-11
        .tm_year = bcd2bin(data[6]) + 100,     // Years since 1900
        .tm_wday = bcd2bin(data[3]) - 1        // 0-6, 0 is Sunday
    };

    *time = mktime(&timeinfo);
    return ESP_OK;
}

esp_err_t ds3231_write_time(time_t time)
{
    struct tm timeinfo;
    localtime_r(&time, &timeinfo);

    // Format: register address, seconds, minutes, hours, day of week, date, month, year
    uint8_t data[8];
    data[0] = DS3231_REG_SECONDS;
    data[1] = bin2bcd(timeinfo.tm_sec);
    data[2] = bin2bcd(timeinfo.tm_min);
    data[3] = bin2bcd(timeinfo.tm_hour);
    data[4] = bin2bcd(timeinfo.tm_wday + 1);  // DS3231: 1-7, tm_wday: 0-6
    data[5] = bin2bcd(timeinfo.tm_mday);
    data[6] = bin2bcd(timeinfo.tm_mon + 1);   // DS3231: 1-12, tm_mon: 0-11
    data[7] = bin2bcd(timeinfo.tm_year - 100);// Years since 2000
    
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, DS3231_I2C_ADDR, 
                                              data, 8, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(DS3231_TAG, "Failed to write to DS3231: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(DS3231_TAG, "Time set on DS3231");
    return ESP_OK;
}
