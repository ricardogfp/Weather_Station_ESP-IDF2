# Waveshare ESP32-S3-Touch-LCD-7 Weather Station+

This project is a feature-rich weather station and smart display for the EWaveshare ESP32-S3-Touch-LCD-7, integrating:

- **Weather Forecast Display**: 7-hour and 7-day weather panel with icons and temperature, using OpenWeatherMap API.
- **Capacitive Touch LCD**: 7-inch ST7701 RGB panel with GT911 touch controller, powered by LVGL for a modern, responsive UI developed in EEZ Studio.
- **Presence Detection**: Human presence sensor (e.g., LD2410C radar) automatically controls the LCD backlight for power saving.
- **RTC Support**: DS3231 real-time clock for accurate timekeeping when offline.
- **WiFi Connectivity**: Fetches weather and time from the internet when available.
- **Home Assistant Integration**: (Optional) Can send sensor/entity data to Home Assistant via MQTT or REST.

## Hardware Requirements
- Waveshare ESP32-S3-Touch-LCD-7
- LD2410C human presence sensor
- DS3231 RTC module

## Software Architecture & Dependencies
- **ESP-IDF >= 5.1.0**
- **LVGL** (v8.3.x)
- **esp_lcd_touch_gt911** (for GT911 touch)
- **Weather client** (OpenWeatherMap API v3)
- **FreeRTOS** for multitasking (UI, sensor, network)

Dependencies are managed via `idf_component.yml` in `main/`:
```yaml
dependencies:
  idf:
    version: ">=5.1.0"
  lvgl/lvgl:
    version: ">8.3.9,<9"
    public: true
  esp_lcd_touch_gt911: "^1"
```

## Build & Flash Instructions
1. **Set up ESP-IDF** (see [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html)).
2. **Configure the project:**
   - Run `idf.py menuconfig` and navigate to `Example Configuration` to set LCD, touch, and sensor options.
3. **Build and flash:**
   - `idf.py set-target esp32s3`
   - `idf.py -p PORT build flash monitor`
4. **Connect hardware** as per the schematic below and power up!

## Features
- Modern, touch-enabled weather UI
- Automatic backlight control based on presence
- RTC fallback for offline clock
- Modular and extendable codebase

---


