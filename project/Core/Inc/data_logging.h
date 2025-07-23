#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "flash_interface.h"

#define FLASH_MEMORY_SIZE_CAMERA     
#define FLASH_MEMORY_SIZE_SENSORS    
#define FLASH_MEMORY_SIZE_ALTIMETER  
#define FLASH_MEMORY_SIZE_GPS        
#define FLASH_MEMORY_SIZE_BATTERY    

#define FLASH_ADDRESS_CAMERA           
#define FLASH_ADDRESS_SENSORS         
#define FLASH_ADDRESS_ALTIMETER     
#define FLASH_ADDRESS_GPS                 
#define FLASH_ADDRESS_BATTERY         

#define DATA_LOGGING_BUFFER_MEMORY_ADDRESS

#define CAMERA 0
#define SENSORS 1
#define ALTIMETER 2
#define GPS 3
#define BATTERY 4
/*
log_data: 
    sends sensor data over serial to a receiver file (TODO: name receiver file once written)
Parameters:
    int data_specifier: specifies {CAMERA, SENSORS, ALTIMETER, GPS, BATTERY} i.e. {0, 1, 2, 3, 4}
Transmission:
    SensorsData structs are transmitted as a sentence of 8 floats separated by spaces
Return:
    returns a true boolean for success and a false boolean for failure 
*/
bool log_data(int data_specifier);