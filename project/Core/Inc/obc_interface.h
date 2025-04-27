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

SensorsData read_sensors(); // writes to memory and returns a SensorsData struct with latest sensor values
SensorsData read_sensor_flash_data(); // returns current flash data
float get_temperature(); // returns current temperature from flash memory, does not update
float get_pressure();

float read_gyroscope_x1();
float read_gyroscope_x2();
float read_gyroscope_x3();

float read_acceleration_x1();
float read_acceleration_x2();
float read_acceleration_x3();

#endif /* SRC_OBC_H_ */