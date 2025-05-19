#ifndef WEATHER_CLIENT_H
#define WEATHER_CLIENT_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

// Maximum string lengths
#define MAX_API_KEY_LEN 40
#define MAX_LAT_LON_LEN 10
#define MAX_WEATHER_DESC_LEN 50
#define MAX_WEATHER_ICON_LEN 8
#define WEATHER_JSON_BUFFER_SIZE 20000
#define MAX_HOURS 7
#define MAX_DAYS 7

// Weather data structures
typedef struct {
    float temperature;
    float temp_max;
    float temp_min;
} WeatherData;

typedef struct {
    WeatherData hours[MAX_HOURS];
} HourlyWeather;

typedef struct {
    WeatherData days[MAX_DAYS];
} DailyWeather;

// Global weather data
extern WeatherData currentWeather;
extern HourlyWeather hourlyWeather;
extern DailyWeather dailyWeather;

// Weather icons
extern char str_Current_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_Weather_Description[MAX_WEATHER_DESC_LEN];
extern char str_1h_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_2h_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_3h_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_4h_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_5h_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_6h_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_7h_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_1d_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_2d_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_3d_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_4d_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_5d_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_6d_weather_icon[MAX_WEATHER_ICON_LEN];
extern char str_7d_weather_icon[MAX_WEATHER_ICON_LEN];

// External configuration
extern const char *openWeatherMapApiKey;
extern const char *lat;
extern const char *lon;

// Status information
extern char ca_Info_Status[32];

// Function prototypes
void init_weather_client(void);
esp_err_t get_data_from_openweathermap(void);
bool wifi_is_connected(void);

// Helper functions for time labels
const char* get_future_hour(int hours_ahead);
const char* get_future_date(int days_ahead);
const char* format_temp(const char* temp);

// Function to get icon code for a specific time period
const char* get_weather_icon_code_current(void);
const char* get_weather_icon_code_hourly(int hour_index);
const char* get_weather_icon_code_daily(int day_index);

#endif /* WEATHER_CLIENT_H */
