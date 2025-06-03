#ifndef PRESENCE_SENSOR_H
#define PRESENCE_SENSOR_H

#define PRESENCE_SENSOR_GPIO 6

#ifdef __cplusplus
extern "C" {
#endif

// Initialize and start the presence sensor task
void presence_sensor_init(void);

#ifdef __cplusplus
}
#endif

#endif // PRESENCE_SENSOR_H
