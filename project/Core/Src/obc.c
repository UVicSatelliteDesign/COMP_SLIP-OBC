#include "obc.h"


float read_temperature();
float read_pressure();
float read_gyroscope_x1();
float read_gyroscope_x2();
float read_gyroscope_x3();

SensorsData get_sensors_data(){
    SensorsData data;
    data.temperature = read_temperature();
    data.pressure = read_pressure();
    data.gyroscope_axis_1 = read_gyroscope_x1();
    data.gyroscope_axis_2 = read_gyroscope_x2();
    data.gyroscope_axis_3 = read_gyroscope_x3();
    return data;
}