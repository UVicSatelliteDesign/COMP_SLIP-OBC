#ifndef SRC_OBC_H_
#define SRC_OBC_H_
#include "stm32h7xx_hal.h"
typedef struct { 		// struct containing sensor data values
    float temperature;        	// temperature, celsius
    float pressure;             	// pressure, unit tbd
    float gyroscope_axis_1;     // gyroscope x axis
    float gyroscope_axis_2;     // gyroscope y axis
    float gyroscope_axis_3;     // gyroscope z axis
    float acceleration_axis_1;  // accelerometer x axis
    float acceleration_axis_2;  // accelerometer y axis
    float acceleration_axis_3;  // accelerometer z axis
    uint32_t magic;             // flash data validation value
} SensorsData;

void init_sensors(); // initialises pins
void save_sensor_data_to_flash(SensorsData *data); // handles saving to flash
SensorsData load_sensor_data_from_flash(); // handles retrieval from flash
SensorsData read_sensors(); // polls sensors, calls save_sensor_data_to_flash, returns sensor data object

float read_temperature(); // poll temperature sensor
float read_pressure();	// poll pressure sensor

float read_gyroscope_x1(); // poll gyroscope
float read_gyroscope_x2();
float read_gyroscope_x3();

float read_acceleration_x1(); // poll accelerometer
float read_acceleration_x2();
float read_acceleration_x3();


#endif /* SRC_OBC_H_ */