#ifndef SRC_OBC_H_
#define SRC_OBC_H_

#include "stm32h7xx_hal.h"

typedef struct {
    float temperature;
    float pressure;
    float gyroscope_axis_1;
    float gyroscope_axis_2;
    float gyroscope_axis_3;
} SensorData;


void init_sensors(void);
SensorsData get_sensors_data(void);
#endif /* SRC_OBC_H_ */