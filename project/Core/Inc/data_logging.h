#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "flash_interface.h"
   
#define FLASH_MEMORY_SIZE_SENSORS     
#define FLASH_MEMORY_SIZE_GPS      //size in bytes  
#define FLASH_MEMORY_SIZE_BATTERY    
         
#define FLASH_ADDRESS_SENSORS   FLASH_SECTOR_SENSORS
#define FLASH_ADDRESS_GPS       FLASH_SECTOR_GPS          
#define FLASH_ADDRESS_BATTERY   FLASH_SECTOR_BATTERY

#define DATA_LOGGING_BUFFER_MEMORY_ADDRESS

#define SENSORS 0
#define GPS 1
#define BATTERY 2
/*
log_data: 
    sends sensor data over serial to a ttc receiver file
Parameters:
    int data_specifier: specifies {SENSORS, GPS, BATTERY}
Cases:
    SENSORS
        Sensor data is printed to serial as a sentence of 10 floats delineated by ', '
        Print order: {one byte specifying data type, OBC temperature, TTC temperature, BMS temperature, gyroscope axis 1, gyroscope axis 2, gyroscope axis 3, acceleration axis 1, acceleration axis 2, acceleration axis 3, altitude}
    GPS
        GPS data is printed from memory to serial as hex, incrementing through the memory by byte
        Print order: {one byte specifying data type, NMEA $GPRMC Sentence}
    BATTERY
        Battery data is printed to serial as a sentence of 6 floats delineated by ', ' 
        Print order: {one byte specifying data type, voltage, current, temperature, state_of_charge, power_usage, estimated_life}
Return:
    returns a true boolean for success and a false boolean for failure 
*/
bool log_data(int data_specifier);