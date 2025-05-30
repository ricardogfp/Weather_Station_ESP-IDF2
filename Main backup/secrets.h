#ifndef SECRETS_H
#define SECRETS_H

// WiFi credentials
#define WIFI_SSID "Kailua"
#define WIFI_PASSWORD "S4mL30v4N3_R"

// Home Assistant configuration
#define HA_URL "https://192.168.6.11:8123"
#define HA_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJlYmU1MDIyYThhODY0MWU4OTM2ZDI4ZGY4NGYwOWNhNSIsImlhdCI6MTc0NTI1OTI5NCwiZXhwIjoyMDYwNjE5Mjk0fQ.-ir8SscnYhs978FR_ebBoFvPiN0Lod-nr5LqT4yYWuo"

// OpenWeatherMap API
#define OPENWEATHER_API_KEY "b2c0beef44dd157da425e740c95d37d0"
#define OPENWEATHER_LAT "40.68"  
#define OPENWEATHER_LON "-3.62" 

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
