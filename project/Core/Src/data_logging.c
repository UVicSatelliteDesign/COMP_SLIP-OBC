#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "flash_interface.h"
#include "data_logging.h"
#include "obc_interface.h"

bool log_data(int data_specifier){
    switch(data_specifier) {
        case CAMERA:
            if(flash_read(DATA_LOGGING_BUFFER_MEMORY_ADDRESS, FLASH_MEMORY_SIZE_CAMERA, FLASH_ADDRESS_CAMERA)){
                //print buffer to serial
                return true;
            }
            return false;
        case SENSORS:
            if(flash_read(DATA_LOGGING_BUFFER_MEMORY_ADDRESS, FLASH_MEMORY_SIZE_SENSORS, FLASH_ADDRESS_SENSORS)){
                SensorsData *SensorsDataPtr = (SensorsData *) DATA_LOGGING_BUFFER_MEMORY_ADDRESS;
                SensorsData data = *SensorsDataPtr;
                //printf modified to send to serial
                printf("%f %f %f %f %f %f %f %f %f %f", data.temperature_obc, data.temperature_ttc, data.temperature_bms, data.gyroscope_axis_1, data.gyroscope_axis_2, data.gyroscope_axis_3, data.acceleration_axis_1, data.acceleration_axis_2, data.acceleration_axis_3, data.altitude);
                return true;
            }
            return false;
        case GPS:
            if(flash_read(DATA_LOGGING_BUFFER_MEMORY_ADDRESS, FLASH_MEMORY_SIZE_GPS, FLASH_ADDRESS_GPS)){
                //print buffer to serial
                return true;
            }
            return false;
        case BATTERY:
            if(flash_read(DATA_LOGGING_BUFFER_MEMORY_ADDRESS, FLASH_MEMORY_SIZE_BATTERY, FLASH_ADDRESS_BATTERY)){
                //print buffer to serial
                return true;
            }
            return false;
        default:
            return false;
        }
}