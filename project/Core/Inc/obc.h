#ifndef SRC_OBC_H_
#define SRC_OBC_H_

#include "stm32h7xx_hal.h"

typedef struct { // struct containing sensor data values
    float temperature;
    float pressure;
    float gyroscope_axis_1;
    float gyroscope_axis_2;
    float gyroscope_axis_3;
} SensorsData;


void init_sensors(void);
SensorsData get_sensors_data(void); //returns a SensorsData struct with current sensor values
#endif /* SRC_OBC_H_ */