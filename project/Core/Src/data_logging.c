#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "flash_interface.h"
#include "data_logging.h"
#include "obc_interface.h"
/*
this will contain functions implementing data logging over serial
functions will be documented in .h file
*/

bool log_data(DataLogType_t data_specifier){
    uint8_t *BufferPtr = malloc(128000);
    if(BufferPtr == NULL){
        free(BufferPtr);
        return false;
    }
    switch(data_specifier) {
        case sensors:
            if(flash_read(BufferPtr, FLASH_MEMORY_SIZE_SENSORS, FLASH_SECTOR_SENSORS, SENSOR_DATA_OFFSET)){
                SensorsData *SensorsDataPtr = (SensorsData *) BufferPtr;
                SensorsData data = *SensorsDataPtr;
                //printf modified to send to serial
                printf("%02x, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f", sensors, data.temperature_obc, data.temperature_ttc, data.temperature_bms, data.gyroscope_axis_1, data.gyroscope_axis_2, data.gyroscope_axis_3, data.acceleration_x, data.acceleration_y, data.acceleration_z, data.altitude);
                free(BufferPtr);
                return true;
            }
            free(BufferPtr);
            return false;
        case gps:
            if(flash_read(BufferPtr, FLASH_MEMORY_SIZE_GPS, FLASH_SECTOR_GPS, 0)){
                uint8_t *GPSPtr = (uint8_t *) BufferPtr;
                printf("%02x, ", gps);
                for(int i = 0; i < FLASH_MEMORY_SIZE_GPS; i++){
                    printf("%02x", GPSPtr);
                    GPSPtr++;
                }
                free(BufferPtr);
                return true;
            }
            free(BufferPtr);
            return false;
        case battery:
            if(flash_read(BufferPtr, FLASH_MEMORY_SIZE_BATTERY, FLASH_SECTOR_BATTERY, 0)){
                BatteryData *BatteryDataPtr = (BatteryData *) BufferPtr;
                BatteryData data = *BatteryDataPtr;
                //printf modified to send to serial
                printf("%02x, %f, %f, %f, %f, %f, %f", battery, data.voltage, data.current, data.state_of_charge, data.power_usage, data.estimated_life);
                free(BufferPtr);
                return true;
            }
            free(BufferPtr);
            return false;
        default:
            free(BufferPtr);
            return false;
        }
}
