
#include "APDS9930.h" // Located in the same (main) directory
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring> // For memcpy

static const char* TAG_APDS = "APDS9930";

#define I2C_MASTER_TIMEOUT_MS       1000 // Timeout for I2C operations
#define APDS9930_ADC_CYCLE_TIME_MS  2.72f // Base ADC integration cycle time in ms

APDS9930::APDS9930(uint8_t i2c_port_num) : 
    _i2c_port_num(i2c_port_num), 
    _is_initialized(false),
    _current_atime_reg_val(DEFAULT_ATIME_REG_VAL),
    _current_again_ctrl_val(DEFAULT_AGAIN) {
}

APDS9930::~APDS9930() {
    // I2C driver is managed externally by the host system.
    // No deinitialization needed here for the I2C bus itself.
}

bool APDS9930::init() {
    if (_is_initialized) {
        ESP_LOGI(TAG_APDS, "APDS-9930 already initialized on I2C port %d.", _i2c_port_num);
        return true;
    }

    ESP_LOGI(TAG_APDS, "Initializing APDS-9930 on pre-configured I2C port %d...", _i2c_port_num);

    // Verify Device ID
    uint8_t device_id = 0;
    if (!readByte(APDS9930_ID, device_id)) {
        ESP_LOGE(TAG_APDS, "Failed to read device ID.");
        return APDS9930_BOOL_ERROR;
    }
    ESP_LOGI(TAG_APDS, "Device ID: 0x%02X", device_id);
    if (device_id != APDS9930_DEVICE_ID) {
        ESP_LOGW(TAG_APDS, "Device ID 0x%02X does not match expected APDS-9930 ID (0x%02X).", device_id, APDS9930_DEVICE_ID);
        // Continue initialization but log a warning. Could return false for stricter check.
    }

    // Disable all features initially to ensure a clean state
    if (!writeByte(APDS9930_ENABLE, 0x00)) {
        ESP_LOGE(TAG_APDS, "Failed to disable all features during initial setup.");
        return APDS9930_BOOL_ERROR;
    }

    // Set default operational parameters
    if (!writeByte(APDS9930_ATIME, DEFAULT_ATIME_REG_VAL)) return APDS9930_BOOL_ERROR;
    _current_atime_reg_val = DEFAULT_ATIME_REG_VAL;

    if (!writeByte(APDS9930_PTIME, DEFAULT_PTIME_REG_VAL)) return APDS9930_BOOL_ERROR;
    if (!writeByte(APDS9930_WTIME, DEFAULT_WTIME_REG_VAL)) return APDS9930_BOOL_ERROR;
    if (!writeByte(APDS9930_PPULSE, DEFAULT_PPULSE_COUNT - 1)) return APDS9930_BOOL_ERROR; // Register value is count-1
    if (!writeByte(APDS9930_POFFSET, DEFAULT_POFFSET_REG_VAL)) return APDS9930_BOOL_ERROR;
    
    uint8_t config_val = DEFAULT_CONFIG_WLONG ? CONFIG_WLONG : 0x00;
    if (!writeByte(APDS9930_CONFIG, config_val)) return APDS9930_BOOL_ERROR;
    
    uint8_t control_val = DEFAULT_PDRIVE | DEFAULT_PDIODE | DEFAULT_PGAIN | DEFAULT_AGAIN;
    if (!writeByte(APDS9930_CONTROL, control_val)) return APDS9930_BOOL_ERROR;
    _current_again_ctrl_val = DEFAULT_AGAIN;

    if (!setALSInterruptThresholds(DEFAULT_AILT_REG_VAL, DEFAULT_AIHT_REG_VAL)) return APDS9930_BOOL_ERROR;
    if (!setProximityInterruptThresholds(DEFAULT_PILT_REG_VAL, DEFAULT_PIHT_REG_VAL)) return APDS9930_BOOL_ERROR;
    if (!writeByte(APDS9930_PERS, DEFAULT_PERS_REG_VAL)) return APDS9930_BOOL_ERROR;

    // Enable power by default after configuration
    if (!enablePower(true)) {
        ESP_LOGE(TAG_APDS, "Failed to enable power on init.");
        return APDS9930_BOOL_ERROR;
    }

    _is_initialized = true;
    ESP_LOGI(TAG_APDS, "APDS-9930 initialized successfully on I2C port %d.", _i2c_port_num);
    return true;
}

uint8_t APDS9930::getDeviceID() {
    if (!_is_initialized) {
        ESP_LOGW(TAG_APDS, "getDeviceID: Sensor not initialized.");
    }
    uint8_t id = 0;
    if (!readByte(APDS9930_ID, id)) {
        return APDS9930_UINT8_ERROR;
    }
    return id;
}

// --- Power and Feature Enable/Disable ---
bool APDS9930::modifyEnableRegister(uint8_t bit_mask, bool enable_bits) {
    uint8_t current_enable_val;
    if (!readByte(APDS9930_ENABLE, current_enable_val)) {
        return APDS9930_BOOL_ERROR;
    }
    if (enable_bits) {
        current_enable_val |= bit_mask;
    } else {
        current_enable_val &= ~bit_mask;
    }
    return writeByte(APDS9930_ENABLE, current_enable_val);
}

bool APDS9930::enablePower(bool enable) {
    return modifyEnableRegister(ENABLE_PON, enable);
}

bool APDS9930::enableProximity(bool enable, bool enable_interrupt) {
    if (!modifyEnableRegister(ENABLE_PEN, enable)) return APDS9930_BOOL_ERROR;
    if (enable) { // Only try to set interrupt enable if main feature is enabled
        if (!modifyEnableRegister(ENABLE_PIEN, enable_interrupt)) return APDS9930_BOOL_ERROR;
    } else { // If disabling proximity, also disable its interrupt
        if (!modifyEnableRegister(ENABLE_PIEN, false)) return APDS9930_BOOL_ERROR;
    }
    return true;
}

bool APDS9930::enableLightSensor(bool enable, bool enable_interrupt) {
    if (!modifyEnableRegister(ENABLE_AEN, enable)) return APDS9930_BOOL_ERROR;
     if (enable) { // Only try to set interrupt enable if main feature is enabled
        if (!modifyEnableRegister(ENABLE_AIEN, enable_interrupt)) return APDS9930_BOOL_ERROR;
    } else { // If disabling ALS, also disable its interrupt
        if (!modifyEnableRegister(ENABLE_AIEN, false)) return APDS9930_BOOL_ERROR;
    }
    return true;
}

bool APDS9930::enableWaitTimer(bool enable) {
    return modifyEnableRegister(ENABLE_WEN, enable);
}

// --- Configuration Getters/Setters ---
bool APDS9930::modifyControlRegister(uint8_t mask, uint8_t value) {
    uint8_t control_val;
    if (!readByte(APDS9930_CONTROL, control_val)) return APDS9930_BOOL_ERROR;
    control_val &= ~mask; // Clear the bits we're about to set
    control_val |= value; // Set the new value
    return writeByte(APDS9930_CONTROL, control_val);
}

uint8_t APDS9930::readControlRegisterField(uint8_t mask, uint8_t shift) {
    uint8_t control_val;
    if (!readByte(APDS9930_CONTROL, control_val)) return APDS9930_UINT8_ERROR;
    return (control_val & mask) >> shift;
}

bool APDS9930::setAmbientLightGain(uint8_t gain_control_val) {
    if (!modifyControlRegister(0x03, gain_control_val & 0x03)) return APDS9930_BOOL_ERROR;
    _current_again_ctrl_val = gain_control_val & 0x03;
    return true;
}

uint8_t APDS9930::getAmbientLightGain() {
    return readControlRegisterField(0x03, 0);
}

bool APDS9930::setProximityGain(uint8_t gain_control_val) {
    return modifyControlRegister(0x0C, gain_control_val & 0x0C);
}

uint8_t APDS9930::getProximityGain() {
    return readControlRegisterField(0x0C, 2) << 2; // Return the masked value directly
}

bool APDS9930::setLEDDriveStrength(uint8_t drive_control_val) {
    return modifyControlRegister(0xC0, drive_control_val & 0xC0);
}

uint8_t APDS9930::getLEDDriveStrength() {
    return readControlRegisterField(0xC0, 6) << 6;
}

bool APDS9930::setProximityDiode(uint8_t diode_control_val) {
    return modifyControlRegister(0x30, diode_control_val & 0x30);
}

uint8_t APDS9930::getProximityDiode() {
    return readControlRegisterField(0x30, 4) << 4;
}

bool APDS9930::setProximityPulseCount(uint8_t count) {
    if (count < 1 || count > 64) {
        ESP_LOGE(TAG_APDS, "Proximity pulse count %d out of range (1-64).", count);
        return APDS9930_BOOL_ERROR;
    }
    return writeByte(APDS9930_PPULSE, count - 1); // Register value is count-1
}

uint8_t APDS9930::getProximityPulseCount() {
    uint8_t ppulse_val;
    if (!readByte(APDS9930_PPULSE, ppulse_val)) return APDS9930_UINT8_ERROR;
    return ppulse_val + 1;
}

// --- Time to Register Value Conversions ---
// ATIME/PTIME/WTIME: RegVal = 256 - (time_ms / 2.72ms_per_cycle)
// time_ms = (256 - RegVal) * 2.72ms_per_cycle
uint8_t APDS9930::timeMsToRegVal(float time_ms, bool is_wtime) {
    float factor = APDS9930_ADC_CYCLE_TIME_MS;
    if (is_wtime) {
        bool wlong_enabled = isWaitLongEnabled();
        if (wlong_enabled == APDS9930_BOOL_ERROR && !_is_initialized) { /* allow during init */ } 
        else if (wlong_enabled) factor *= 12.0f;
    }
    if (time_ms < factor) time_ms = factor; // Min time is 1 cycle
    float max_time = 256.0f * factor;
    if (time_ms > max_time) time_ms = max_time; // Max time is 256 cycles
    
    uint8_t reg_val = 255 - (uint8_t)((time_ms / factor) -1.0f) ; // 255 for 1 cycle, 0 for 256 cycles
    return reg_val;
}

float APDS9930::regValToTimeMs(uint8_t reg_val, bool is_wtime) {
    float factor = APDS9930_ADC_CYCLE_TIME_MS;
     if (is_wtime) {
        bool wlong_enabled = isWaitLongEnabled();
        if (wlong_enabled == APDS9930_BOOL_ERROR && !_is_initialized) { /* allow during init */ } 
        else if (wlong_enabled) factor *= 12.0f;
    }
    return (256.0f - (float)reg_val) * factor;
}

bool APDS9930::setAmbientLightIntegrationTime(float time_ms) {
    uint8_t atime_reg = timeMsToRegVal(time_ms);
    if (!writeByte(APDS9930_ATIME, atime_reg)) return APDS9930_BOOL_ERROR;
    _current_atime_reg_val = atime_reg;
    return true;
}

float APDS9930::getAmbientLightIntegrationTime() {
    uint8_t atime_reg;
    if (!readByte(APDS9930_ATIME, atime_reg)) return APDS9930_FLOAT_ERROR;
    return regValToTimeMs(atime_reg);
}

bool APDS9930::setProximityIntegrationTime(float time_ms) {
    return writeByte(APDS9930_PTIME, timeMsToRegVal(time_ms));
}

float APDS9930::getProximityIntegrationTime() {
    uint8_t ptime_reg;
    if (!readByte(APDS9930_PTIME, ptime_reg)) return APDS9930_FLOAT_ERROR;
    return regValToTimeMs(ptime_reg);
}

bool APDS9930::setWaitTime(float time_ms) {
    return writeByte(APDS9930_WTIME, timeMsToRegVal(time_ms, true));
}

float APDS9930::getWaitTime() {
    uint8_t wtime_reg;
    if (!readByte(APDS9930_WTIME, wtime_reg)) return APDS9930_FLOAT_ERROR;
    return regValToTimeMs(wtime_reg, true);
}

bool APDS9930::setWaitLongEnabled(bool enable) {
    uint8_t config_val;
    if (!readByte(APDS9930_CONFIG, config_val)) return APDS9930_BOOL_ERROR;
    if (enable) {
        config_val |= CONFIG_WLONG;
    } else {
        config_val &= ~CONFIG_WLONG;
    }
    return writeByte(APDS9930_CONFIG, config_val);
}

bool APDS9930::isWaitLongEnabled() {
    uint8_t config_val;
    if (!readByte(APDS9930_CONFIG, config_val)) return APDS9930_BOOL_ERROR; // Indicate error
    return (config_val & CONFIG_WLONG) ? true : false;
}

// --- Interrupt Configuration ---
bool APDS9930::setALSInterruptThresholds(uint16_t low, uint16_t high) {
    uint8_t data[4];
    data[0] = low & 0xFF;
    data[1] = (low >> 8) & 0xFF;
    data[2] = high & 0xFF;
    data[3] = (high >> 8) & 0xFF;
    return writeBlock(APDS9930_AILTL, data, 4);
}

bool APDS9930::getALSInterruptLowThreshold(uint16_t &threshold) {
    uint8_t data[2];
    if (!readBlock(APDS9930_AILTL, data, 2)) return APDS9930_BOOL_ERROR;
    threshold = (data[1] << 8) | data[0];
    return true;
}

bool APDS9930::getALSInterruptHighThreshold(uint16_t &threshold) {
    uint8_t data[2];
    if (!readBlock(APDS9930_AIHTL, data, 2)) return APDS9930_BOOL_ERROR;
    threshold = (data[1] << 8) | data[0];
    return true;
}

// Proximity data is 10-bit for APDS-9930 (PDATAL, PDATAH bits 0-1)
// Thresholds PILT, PIHT are also 10-bit (PILTL/H, PIHTL/H)
bool APDS9930::setProximityInterruptThresholds(uint16_t low, uint16_t high) {
    low &= 0x03FF; // Ensure 10-bit
    high &= 0x03FF;
    uint8_t data[4];
    data[0] = low & 0xFF;
    data[1] = (low >> 8) & 0x03;
    data[2] = high & 0xFF;
    data[3] = (high >> 8) & 0x03;
    return writeBlock(APDS9930_PILTL, data, 4);
}

bool APDS9930::getProximityInterruptLowThreshold(uint16_t &threshold) {
    uint8_t data[2];
    if (!readBlock(APDS9930_PILTL, data, 2)) return APDS9930_BOOL_ERROR;
    threshold = ((data[1] & 0x03) << 8) | data[0]; // 10-bit value
    return true;
}

bool APDS9930::getProximityInterruptHighThreshold(uint16_t &threshold) {
    uint8_t data[2];
    if (!readBlock(APDS9930_PIHTL, data, 2)) return APDS9930_BOOL_ERROR;
    threshold = ((data[1] & 0x03) << 8) | data[0];
    return true;
}

// PERS: bits 7-4 for ALS (APERS), bits 3-0 for Prox (PPERS)
// Values 0-15. 0=Every ADC cycle, 1=1, 2=2, ..., 15=15 consecutive for interrupt.
bool APDS9930::setInterruptPersistence(uint8_t als_persistence, uint8_t prox_persistence) {
    als_persistence &= 0x0F;
    prox_persistence &= 0x0F;
    uint8_t pers_val = (als_persistence << 4) | prox_persistence;
    return writeByte(APDS9930_PERS, pers_val);
}

bool APDS9930::getInterruptPersistence(uint8_t &als_persistence, uint8_t &prox_persistence) {
    uint8_t pers_val;
    if (!readByte(APDS9930_PERS, pers_val)) return APDS9930_BOOL_ERROR;
    als_persistence = (pers_val >> 4) & 0x0F;
    prox_persistence = pers_val & 0x0F;
    return true;
}

// --- Interrupt Clearing & Status ---
bool APDS9930::clearALSInterrupt() {
    return writeSpecialFunction(SF_ALS_INT_CLR);
}

bool APDS9930::clearProximityInterrupt() {
    return writeSpecialFunction(SF_PROX_INT_CLR);
}

bool APDS9930::clearAllInterrupts() {
    return writeSpecialFunction(SF_ALL_INTS_CLR);
}

// STATUS register (0x13, accessed via 0x93)
// Bit 0: AVALID - ALS Valid.
// Bit 1: PVALID - Proximity Valid.
// Bit 4: AINT - ALS Interrupt.
// Bit 5: PINT - Proximity Interrupt.
bool APDS9930::isProximityInterruptPending() {
    uint8_t status;
    if (!readByte(APDS9930_STATUS, status)) return APDS9930_BOOL_ERROR;
    return (status & (1 << 5)) ? true : false;
}

bool APDS9930::isALSInterruptPending() {
    uint8_t status;
    if (!readByte(APDS9930_STATUS, status)) return APDS9930_BOOL_ERROR;
    return (status & (1 << 4)) ? true : false;
}

// --- Data Reading ---
bool APDS9930::readProximityValue(uint16_t &value) {
    uint8_t data[2]; // PDATAL, PDATAH
    if (!readBlock(APDS9930_PDATAL, data, 2)) return APDS9930_BOOL_ERROR;
    value = ((data[1] & 0x03) << 8) | data[0]; // 10-bit value
    return true;
}

bool APDS9930::readALSData(uint16_t &ch0_clear_value, uint16_t &ch1_ir_value) {
    uint8_t data[4]; // CDATAL, CDATAH, IRDATAL, IRDATAH
    if (!readBlock(APDS9930_CDATAL, data, 4)) return APDS9930_BOOL_ERROR;
    ch0_clear_value = (data[1] << 8) | data[0];
    ch1_ir_value = (data[3] << 8) | data[2];
    return true;
}

// Lux calculation based on APDS-9930 Application Note AN000173 and common library implementations.
// This formula is an empirical approximation and may need tuning with specific coefficients for best accuracy.
bool APDS9930::readLux(float &lux) {
    uint16_t ch0, ch1;
    if (!readALSData(ch0, ch1)) {
        lux = APDS9930_FLOAT_ERROR;
        return APDS9930_BOOL_ERROR;
    }

    // Retrieve current ATIME and AGAIN for calculation (these should be up-to-date in member vars)
    float atime_ms = regValToTimeMs(_current_atime_reg_val);
    float again_x_factor;
    switch (_current_again_ctrl_val) {
        case CONTROL_AGAIN_1X:  again_x_factor = 1.0f; break;
        case CONTROL_AGAIN_8X:  again_x_factor = 8.0f; break;
        case CONTROL_AGAIN_16X: again_x_factor = 16.0f; break;
        case CONTROL_AGAIN_120X: again_x_factor = 120.0f; break;
        default: again_x_factor = 1.0f; ESP_LOGW(TAG_APDS, "readLux: Unknown AGAIN value 0x%X", _current_again_ctrl_val); break;
    }

    // Counts Per Lux (CPL) calculation - this is critical and device/setup dependent
    // CPL = (ATIME_ms * AGAINx) / (GA * DF)
    // GA (Glass Attenuation) = 1.0 for no glass/cover. Adjust if sensor is covered.
    // DF (Device Factor) for APDS-9930 is typically around 52.
    const float GA = 1.0f;
    const float DF = 52.0f;
    float cpl = (atime_ms * again_x_factor) / (GA * DF);
    if (cpl == 0) { // Avoid division by zero
        lux = 0.0f;
        return true;
    }

    float ch0_f = (float)ch0;
    float ch1_f = (float)ch1;

    // IR Rejection Coefficients (these are typical, may need tuning)
    // These coefficients are used to form two Intermediate Lux values (IAC1, IAC2)
    // Lux = max(0, IAC1, IAC2) / CPL  OR  Lux = (CH0 - B*CH1)/CPL or Lux = (D*CH0 - CH1)/CPL
    // A common empirical formula found in many libraries (e.g., Adafruit, Sparkfun for similar sensors)
    // is based on the CH1/CH0 ratio.

    if (ch0_f == 0) { // No light or error reading CH0
        lux = 0.0f;
        return true;
    }
    float ratio = ch1_f / ch0_f;

    // Empirical lux formula (common for APDS-99xx family, may need tuning for specific APDS-9930 variant/conditions)
    if (ratio <= 0.50f) {
        lux = (0.0304f * ch0_f) - (0.062f * ch0_f * std::pow(ratio, 1.4f));
    } else if (ratio <= 0.61f) {
        lux = (0.0224f * ch0_f) - (0.031f * ch1_f);
    } else if (ratio <= 0.80f) {
        lux = (0.0128f * ch0_f) - (0.0153f * ch1_f);
    } else if (ratio <= 1.30f) {
        lux = (0.00146f * ch0_f) - (0.00112f * ch1_f);
    } else { // Ratio > 1.30, or ch1_f is significantly larger than ch0_f (very high IR content or error)
        lux = 0.0f;
    }

    if (lux < 0) lux = 0.0f; // Lux cannot be negative

    return true;
}

// --- Status Register Checks ---
bool APDS9930::isProximityDataValid() {
    uint8_t status;
    if (!readByte(APDS9930_STATUS, status)) return APDS9930_BOOL_ERROR;
    return (status & (1 << 1)) ? true : false; // PVALID bit
}

bool APDS9930::isALSDataValid() {
    uint8_t status;
    if (!readByte(APDS9930_STATUS, status)) return APDS9930_BOOL_ERROR;
    return (status & (1 << 0)) ? true : false; // AVALID bit
}

// --- Low-level I2C communication functions (using ESP-IDF I2C master API) ---
// Note: APDS-9930 datasheet specifies a Command Register (CMD bit 7 of address byte).
// CMD=1 for register access (read/write). CMD=0 is for SMBus block read/write (not used here).
// All register addresses defined in .h already have CMD bit (0x80) set.

bool APDS9930::writeByte(uint8_t reg_addr_with_cmd, uint8_t data) {
    uint8_t buffer[2] = {reg_addr_with_cmd, data};
    esp_err_t err = i2c_master_write_to_device((i2c_port_t)_i2c_port_num, APDS9930_I2C_ADDR, 
                                               buffer, sizeof(buffer), 
                                               pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    if (err != ESP_OK) {
        ESP_LOGE(TAG_APDS, "I2C writeByte to reg 0x%02X failed: %s (0x%X)", reg_addr_with_cmd, esp_err_to_name(err), err);
        return APDS9930_BOOL_ERROR;
    }
    return true;
}

bool APDS9930::writeBlock(uint8_t reg_addr_with_cmd, const uint8_t *data, size_t len) {
    if (len == 0) return true;
    if (len > 31) { // I2C buffer might be limited (e.g. 32 bytes including reg addr)
        ESP_LOGE(TAG_APDS, "writeBlock: len %u too long.", len);
        return APDS9930_BOOL_ERROR;
    }

    uint8_t buffer[len + 1];
    buffer[0] = reg_addr_with_cmd;
    memcpy(buffer + 1, data, len);

    esp_err_t err = i2c_master_write_to_device((i2c_port_t)_i2c_port_num, APDS9930_I2C_ADDR, 
                                               buffer, len + 1, 
                                               pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    if (err != ESP_OK) {
        ESP_LOGE(TAG_APDS, "I2C writeBlock to reg 0x%02X failed: %s (0x%X)", reg_addr_with_cmd, esp_err_to_name(err), err);
        return APDS9930_BOOL_ERROR;
    }
    return true;
}

bool APDS9930::readByte(uint8_t reg_addr_with_cmd, uint8_t &data) {
    esp_err_t err = i2c_master_write_read_device((i2c_port_t)_i2c_port_num, APDS9930_I2C_ADDR, 
                                               &reg_addr_with_cmd, 1, // Write the register address (with CMD bit)
                                               &data, 1,             // Read 1 byte of data
                                               pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    if (err != ESP_OK) {
        ESP_LOGE(TAG_APDS, "I2C readByte from reg 0x%02X failed: %s (0x%X)", reg_addr_with_cmd, esp_err_to_name(err), err);
        return APDS9930_BOOL_ERROR;
    }
    return true;
}

bool APDS9930::readBlock(uint8_t reg_addr_with_cmd, uint8_t *data, size_t len) {
    if (len == 0) return true;
    esp_err_t err = i2c_master_write_read_device((i2c_port_t)_i2c_port_num, APDS9930_I2C_ADDR, 
                                               &reg_addr_with_cmd, 1, 
                                               data, len, 
                                               pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    if (err != ESP_OK) {
        ESP_LOGE(TAG_APDS, "I2C readBlock from reg 0x%02X failed: %s (0x%X)", reg_addr_with_cmd, esp_err_to_name(err), err);
        return APDS9930_BOOL_ERROR;
    }
    return true;
}

// Special functions (like interrupt clear) are written to the COMMAND register (0x80)
// The value written is the special function code (e.g., 0xE7 to clear all interrupts).
bool APDS9930::writeSpecialFunction(uint8_t sf_command_value) {
    // The command register itself is 0x80. We write the SF code to it.
    return writeByte(SF_COMMAND_REGISTER, sf_command_value);
}