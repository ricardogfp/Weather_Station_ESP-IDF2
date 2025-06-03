#ifndef SECRETS_H
#define SECRETS_H

// WiFi credentials
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "Password"

// Home Assistant configuration
#define HA_URL "https://localhost:8123"
#define HA_TOKEN "HA_TOKEN"

// OpenWeatherMap API
#define OPENWEATHER_API_KEY "API_KEY"
#define OPENWEATHER_LAT "LAT"  
#define OPENWEATHER_LON "LON" 
#define OPENWEATHER_CITY "City" 

//Time zone
/* Set timezone to Central European Time (CET/CEST)
* Format: setenv("TZ", "[std][offset][dst][,start[/time],end[/time]]", overwrite)
* where:
*   std = standard time zone abbreviation (e.g., CET)
*   offset = hours east of UTC (e.g., -1 means UTC+1)
*   dst = daylight saving time zone abbreviation (e.g., CEST)
*   start/end = DST transition dates (M=month.week.day)
*   time = transition time in hours
*/
#define TIMEZONE_STRING "CET-1CEST,M3.5.0/2,M10.5.0/3"

#endif // SECRETS_H
