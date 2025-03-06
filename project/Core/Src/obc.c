#include "obc.h"


float read_temperature(); // return current temperature value in degrees celsius
float read_pressure();    // return current pressure value 
float read_gyroscope_x1();// return current axis_1 value 
float read_gyroscope_x2();// return current axis_2 value
float read_gyroscope_x3();// return current axis_3 value

SensorsData get_sensors_data(){ //returns a SensorsData struct with current sensor values
    SensorsData data;
    data.temperature = read_temperature();
    data.pressure = read_pressure();
    data.gyroscope_axis_1 = read_gyroscope_x1();
    data.gyroscope_axis_2 = read_gyroscope_x2();
    data.gyroscope_axis_3 = read_gyroscope_x3();
    return data;
}