#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "flash_interface.h"
#include "obc_interface.h"

typedef enum{
    sensors = 0,
    gps = 1,
    battery = 2
}DataLogType_t;

///////////////////////////////
extern int FLASH_MEMORY_SIZE_SENSORS;
extern int FLASH_MEMORY_SIZE_GPS;      //size in bytes: 79 characters at 1byte/char
extern int FLASH_MEMORY_SIZE_BATTERY;
/////////////^^^These size variables need declaration/////////////////


/*
log_data: 
    logs data over serial to a ttc receiver file
Parameters:
    DataLogType_t data_specifier: specifies {sensors, gps, battery}
Cases:
    sensors
        Sensor data is printed to serial as a sentence of 10 floats delineated by ', '
        Print order: {one byte specifying data type, OBC temperature, TTC temperature, BMS temperature, gyroscope axis 1, gyroscope axis 2, gyroscope axis 3, acceleration axis 1, acceleration axis 2, acceleration axis 3, altitude}
    gps
        GPS data is printed from memory to serial as hex, incrementing through the memory by byte
        Print order: {one byte specifying data type, NMEA $GPRMC Sentence}
    battery
        Battery data is printed to serial as a sentence of 6 floats delineated by ', ' 
        Print order: {one byte specifying data type, voltage, current, temperature, state_of_charge, power_usage, estimated_life}
Return:
    returns a true boolean for success and a false boolean for failure 
*/
bool log_data(DataLogType_t data_specifier);
