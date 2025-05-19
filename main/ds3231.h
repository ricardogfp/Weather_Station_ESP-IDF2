#pragma once

#include <time.h>
#include <stdbool.h>
#include "esp_err.h"

// DS3231 I2C address
#define DS3231_I2C_ADDR 0x68

// DS3231 register addresses
#define DS3231_REG_SECONDS 0x00
#define DS3231_REG_MINUTES 0x01
#define DS3231_REG_HOURS   0x02
#define DS3231_REG_DAY     0x03
#define DS3231_REG_DATE    0x04
#define DS3231_REG_MONTH   0x05
#define DS3231_REG_YEAR    0x06
#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_STATUS  0x0F
#define DS3231_REG_TEMP_MSB 0x11
#define DS3231_REG_TEMP_LSB 0x12

/**
 * @brief Initialize DS3231 module
 * 
 * @return ESP_OK if successful
 */
esp_err_t ds3231_init(void);

/**
 * @brief Read time from DS3231
 * 
 * @param time Pointer to time_t variable to store the time
 * @return ESP_OK if successful
 */
esp_err_t ds3231_read_time(time_t *time);

/**
 * @brief Write time to DS3231
 * 
 * @param time Time value to write
 * @return ESP_OK if successful
 */
esp_err_t ds3231_write_time(time_t time);

/**
 * @brief Convert BCD to binary
 * 
 * @param val BCD value
 * @return Binary value
 */
uint8_t bcd2bin(uint8_t val);

/**
 * @brief Convert binary to BCD
 * 
 * @param val Binary value
 * @return BCD value
 */
uint8_t bin2bcd(uint8_t val);
