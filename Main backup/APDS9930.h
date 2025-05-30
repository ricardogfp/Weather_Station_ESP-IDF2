// Original Arduino Library: https://github.com/sparkfun/SparkFun_APDS-9960_Sensor_Arduino_Library (APDS-9960, adapted for APDS-9930)
// And https://github.com/Depau/APDS9930 (More specific to APDS-9930)
// Adapted for ESP-IDF, to be placed in the main project directory.
#ifndef APDS9930_H
#define APDS9930_H

#include <stdint.h>
#include <stdbool.h>
#include <cmath> // For std::pow in lux calculation, and NAN

// APDS-9930 I2C address
#define APDS9930_I2C_ADDR       0x39

// Error code for returned values
#define APDS9930_UINT8_ERROR    0xFF // Using 0xFF as a generic error for uint8_t returns
#define APDS9930_FLOAT_ERROR    NAN  // For float returns
#define APDS9930_BOOL_ERROR     false // For bool returns indicating failure

// APDS-9930 Device ID (Register 0x12, value 0x39)
#define APDS9930_DEVICE_ID      0x39

// APDS-9930 register addresses
#define APDS9930_ENABLE         0x80 // Enable states and interrupts (CMD=1, ADDR=0x00)
#define APDS9930_ATIME          0x81 // ALS ADC Integration Time (CMD=1, ADDR=0x01)
#define APDS9930_PTIME          0x82 // Proximity ADC Time (CMD=1, ADDR=0x02)
#define APDS9930_WTIME          0x83 // Wait Time (CMD=1, ADDR=0x03)
#define APDS9930_AILTL          0x84 // ALS Interrupt Low Threshold Low Byte
#define APDS9930_AILTH          0x85 // ALS Interrupt Low Threshold High Byte
#define APDS9930_AIHTL          0x86 // ALS Interrupt High Threshold Low Byte
#define APDS9930_AIHTH          0x87 // ALS Interrupt High Threshold High Byte
#define APDS9930_PILTL          0x88 // Proximity Interrupt Low Threshold Low Byte
#define APDS9930_PILTH          0x89 // Proximity Interrupt Low Threshold High Byte
#define APDS9930_PIHTL          0x8A // Proximity Interrupt High Threshold Low Byte
#define APDS9930_PIHTH          0x8B // Proximity Interrupt High Threshold High Byte
#define APDS9930_PERS           0x8C // Interrupt Persistence Filters
#define APDS9930_CONFIG         0x8D // Configuration Register (WLONG)
#define APDS9930_PPULSE         0x8E // Proximity Pulse Count and Length
#define APDS9930_CONTROL        0x8F // Control Register (LED Drive, PGAIN, AGAIN, PDIODE)
#define APDS9930_ID             0x92 // Device ID Register
#define APDS9930_STATUS         0x93 // Device Status Register
#define APDS9930_CDATAL         0x94 // CH0 ADC Low Data Register (Clear)
#define APDS9930_CDATAH         0x95 // CH0 ADC High Data Register (Clear)
#define APDS9930_IRDATAL        0x96 // CH1 ADC Low Data Register (IR)
#define APDS9930_IRDATAH        0x97 // CH1 ADC High Data Register (IR)
#define APDS9930_PDATAL         0x98 // Proximity ADC Low Data Register
#define APDS9930_PDATAH         0x99 // Proximity ADC High Data Register
#define APDS9930_POFFSET        0x9E // Proximity Offset Register (CMD=1, ADDR=0x1E)
// POFFSET (0x9E) is sometimes mentioned but might be specific to variants or older datasheets.
// Using Command bit (MSB=1) for register access as per datasheet for repeated byte protocol.

// Bit fields for ENABLE register (0x00, accessed via 0x80)
#define ENABLE_PON            (1 << 0) // Power ON
#define ENABLE_AEN            (1 << 1) // ALS Enable
#define ENABLE_PEN            (1 << 2) // Proximity Enable
#define ENABLE_WEN            (1 << 3) // Wait Enable
#define ENABLE_AIEN           (1 << 4) // ALS Interrupt Enable
#define ENABLE_PIEN           (1 << 5) // Proximity Interrupt Enable
// Bit 6 is reserved

// Bit fields for CONFIG register (0x0D, accessed via 0x8D)
#define CONFIG_WLONG          (1 << 1) // Wait Long (Increases wait time by 12x)

// Bit fields for CONTROL register (0x0F, accessed via 0x8F)
// LED Drive Strength (PDRIVE) - Bits 7:6
#define CONTROL_PDRIVE_100MA            (0 << 6)
#define CONTROL_PDRIVE_50MA             (1 << 6)
#define CONTROL_PDRIVE_25MA             (2 << 6)
#define CONTROL_PDRIVE_12_5MA           (3 << 6)
// Proximity Diode Select (PDIODE) - Bits 5:4
#define CONTROL_PDIODE_RESERVED         (0 << 4) // Reserved
#define CONTROL_PDIODE_IR_LED_ONLY      (1 << 4) // Use IR LED for proximity
#define CONTROL_PDIODE_CH1_DIODE_ONLY   (2 << 4) // Use CH1 Photodiode for proximity (Datasheet: Proximity detection uses the CH1 photodiode)
#define CONTROL_PDIODE_BOTH_PROX_PULSES (3 << 4) // Both LEDs for proximity pulses
// Proximity Gain Control (PGAIN) - Bits 3:2
#define CONTROL_PGAIN_1X                (0 << 2)
#define CONTROL_PGAIN_2X                (1 << 2)
#define CONTROL_PGAIN_4X                (2 << 2)
#define CONTROL_PGAIN_8X                (3 << 2)
// Ambient Light Sensor Gain Control (AGAIN) - Bits 1:0
#define CONTROL_AGAIN_1X                (0 << 0)
#define CONTROL_AGAIN_8X                (1 << 0)
#define CONTROL_AGAIN_16X               (2 << 0)
#define CONTROL_AGAIN_120X              (3 << 0)

// Special Function Command Codes (written to Command Register with CMD=1 and SF bit set)
// These are written to a command register which is formed by setting CMD=1 (0x80) and then the special function code.
// For APDS-9930, these are typically written to the ENABLE register (0x00) with the special function bits (7:5) set.
// Example: Clear Proximity Interrupt = 0b11100101 = 0xE5. This is written to the command register (0x80 | 0x00 usually implies ENABLE)
// The datasheet says "The Special Function commands are entered by writing the command code to the COMMAND register.
// The COMMAND register is write-only and is accessed when the CMD bit (bit 7) of the address byte is set to 1 AND the address bits (6:0) are 00h."
// So, the command register is 0x80. The value written to it is the special function code.
#define SF_COMMAND_REGISTER     0x80 // Command register address for special functions
#define SF_PROX_INT_CLR         0xE5 // Clear Proximity Interrupt
#define SF_ALS_INT_CLR          0xE6 // Clear ALS Interrupt
#define SF_ALL_INTS_CLR         0xE7 // Clear All Interrupts

// On/Off definitions for enable/disable methods
#define SENSOR_OFF                     0
#define SENSOR_ON                      1

// Default values for initialization
#define DEFAULT_ATIME_REG_VAL   0xFF    // 2.72ms (256 - 1 cycle = 255 -> 0xFF)
#define DEFAULT_PTIME_REG_VAL   0xFF    // 2.72ms
#define DEFAULT_WTIME_REG_VAL   0xFF    // 2.72ms
#define DEFAULT_PPULSE_COUNT    8       // Number of prox pulses (1-64). Register value is count-1.
#define DEFAULT_POFFSET_REG_VAL 0x00    // Proximity offset 0
#define DEFAULT_CONFIG_WLONG    false   // WLONG bit in CONFIG register
#define DEFAULT_PDRIVE          CONTROL_PDRIVE_100MA
#define DEFAULT_PDIODE          CONTROL_PDIODE_CH1_DIODE_ONLY
#define DEFAULT_PGAIN           CONTROL_PGAIN_4X
#define DEFAULT_AGAIN           CONTROL_AGAIN_1X
#define DEFAULT_AILT_REG_VAL    0xFFFF  // ALS Interrupt low threshold (effectively disabled)
#define DEFAULT_AIHT_REG_VAL    0       // ALS Interrupt high threshold (effectively disabled)
#define DEFAULT_PILT_REG_VAL    0       // Proximity interrupt low threshold (10-bit)
#define DEFAULT_PIHT_REG_VAL    1023    // Proximity interrupt high threshold (max 10-bit value)
#define DEFAULT_PERS_REG_VAL    0x22    // ALS: 2 cons. values, PROX: 2 cons. values for interrupt

class APDS9930 {
public:
    APDS9930(uint8_t i2c_port_num);
    ~APDS9930();

    bool init(); // Assumes I2C bus is already configured by the host system
    uint8_t getDeviceID();

    // Power and Feature Enable/Disable
    bool enablePower(bool enable = true);
    bool enableProximity(bool enable = true, bool enable_interrupt = false);
    bool enableLightSensor(bool enable = true, bool enable_interrupt = false);
    bool enableWaitTimer(bool enable = true);

    // Configuration Getters/Setters
    bool setAmbientLightGain(uint8_t gain_control_val); // Use CONTROL_AGAIN_xX values
    uint8_t getAmbientLightGain(); // Returns CONTROL_AGAIN_xX value or APDS9930_UINT8_ERROR
    bool setProximityGain(uint8_t gain_control_val); // Use CONTROL_PGAIN_xX values
    uint8_t getProximityGain(); // Returns CONTROL_PGAIN_xX value or APDS9930_UINT8_ERROR
    bool setLEDDriveStrength(uint8_t drive_control_val); // Use CONTROL_PDRIVE_xMA values
    uint8_t getLEDDriveStrength(); // Returns CONTROL_PDRIVE_xMA value or APDS9930_UINT8_ERROR
    bool setProximityDiode(uint8_t diode_control_val); // Use CONTROL_PDIODE_x values
    uint8_t getProximityDiode(); // Returns CONTROL_PDIODE_x value or APDS9930_UINT8_ERROR
    bool setProximityPulseCount(uint8_t count); // Actual count from 1-64
    uint8_t getProximityPulseCount(); // Returns actual count or APDS9930_UINT8_ERROR

    bool setAmbientLightIntegrationTime(float time_ms); // Time in milliseconds
    float getAmbientLightIntegrationTime(); // Returns time in ms or APDS9930_FLOAT_ERROR
    bool setProximityIntegrationTime(float time_ms); // Time in milliseconds
    float getProximityIntegrationTime(); // Returns time in ms or APDS9930_FLOAT_ERROR
    bool setWaitTime(float time_ms); // Time in milliseconds, considers WLONG
    float getWaitTime(); // Returns time in ms or APDS9930_FLOAT_ERROR
    bool setWaitLongEnabled(bool enable);
    bool isWaitLongEnabled();

    // Interrupt Configuration
    bool setALSInterruptThresholds(uint16_t low, uint16_t high); // Raw ADC values
    bool getALSInterruptLowThreshold(uint16_t &threshold);
    bool getALSInterruptHighThreshold(uint16_t &threshold);
    bool setProximityInterruptThresholds(uint16_t low, uint16_t high); // Raw 10-bit ADC values (0-1023)
    bool getProximityInterruptLowThreshold(uint16_t &threshold);
    bool getProximityInterruptHighThreshold(uint16_t &threshold);
    bool setInterruptPersistence(uint8_t als_persistence, uint8_t prox_persistence); // Values 0-15
    bool getInterruptPersistence(uint8_t &als_persistence, uint8_t &prox_persistence);

    // Interrupt Clearing & Status
    bool clearALSInterrupt();
    bool clearProximityInterrupt();
    bool clearAllInterrupts();
    bool isProximityInterruptPending();
    bool isALSInterruptPending();

    // Data Reading
    bool readProximityValue(uint16_t &value); // Returns raw 10-bit proximity ADC value
    bool readALSData(uint16_t &ch0_clear_value, uint16_t &ch1_ir_value);
    bool readLux(float &lux); // Calculates lux based on current settings and CH0/CH1 data

    // Status Register Checks
    bool isProximityDataValid();
    bool isALSDataValid();

private:
    uint8_t _i2c_port_num;
    bool _is_initialized;
    uint8_t _current_atime_reg_val; // Store current ATIME register value for lux calculation
    uint8_t _current_again_ctrl_val; // Store current AGAIN control value for lux calculation

    // Low-level I2C communication functions (using ESP-IDF I2C master API)
    bool writeByte(uint8_t reg_addr_with_cmd, uint8_t data);
    bool writeBlock(uint8_t reg_addr_with_cmd, const uint8_t *data, size_t len);
    bool readByte(uint8_t reg_addr_with_cmd, uint8_t &data);
    bool readBlock(uint8_t reg_addr_with_cmd, uint8_t *data, size_t len);
    bool writeSpecialFunction(uint8_t sf_command_value);

    // Helper to set/clear specific bits in the ENABLE register
    bool modifyEnableRegister(uint8_t bit_mask, bool enable_bits);
    // Helper to read/modify/write CONTROL register fields
    bool modifyControlRegister(uint8_t mask, uint8_t value);
    uint8_t readControlRegisterField(uint8_t mask, uint8_t shift);

    // Conversion helpers
    uint8_t timeMsToRegVal(float time_ms, bool is_wtime = false);
    float regValToTimeMs(uint8_t reg_val, bool is_wtime = false);
};

#endif // APDS9930_H