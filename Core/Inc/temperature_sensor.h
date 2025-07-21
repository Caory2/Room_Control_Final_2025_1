#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

float temperature_sensor_read(void);

#ifdef __cplusplus
}
#endif

#endif // TEMPERATURE_SENSOR_H