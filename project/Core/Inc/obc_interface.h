#ifndef SRC_OBC_H_
#define SRC_OBC_H_

#include "stm32h7xx_hal.h"
typedef struct { // struct containing sensor data values
    float temperature;
    float pressure;
    float gyroscope_axis_1;
    float gyroscope_axis_2;
    float gyroscope_axis_3;
    float acceleration_axis_1;
    float acceleration_axis_2;
    float acceleration_axis_3;
} SensorsData;

SensorsData get_sensors_data(); //returns a SensorsData struct with current sensor values
float get_temperature();
float get_pressure();

#endif /* SRC_OBC_H_ */