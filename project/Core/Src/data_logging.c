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
        case SENSORS:
            if(flash_read(DATA_LOGGING_BUFFER_MEMORY_ADDRESS, FLASH_MEMORY_SIZE_SENSORS, FLASH_ADDRESS_SENSORS)){
                SensorsData *SensorsDataPtr = (SensorsData *) DATA_LOGGING_BUFFER_MEMORY_ADDRESS;
                SensorsData data = *SensorsDataPtr;
                //printf modified to send to serial
                printf("%f, %f, %f, %f, %f, %f, %f, %f, %f, %f", data.temperature_obc, data.temperature_ttc, data.temperature_bms, data.gyroscope_axis_1, data.gyroscope_axis_2, data.gyroscope_axis_3, data.acceleration_axis_1, data.acceleration_axis_2, data.acceleration_axis_3, data.altitude);
                return true;
            }
            return false;
        case GPS:
            if(flash_read(DATA_LOGGING_BUFFER_MEMORY_ADDRESS, FLASH_MEMORY_SIZE_GPS, FLASH_ADDRESS_GPS)){
                uint8_t *GPSPtr = (uint8_t *) DATA_LOGGING_BUFFER_MEMORY_ADDRESS;
                for(int i = 0; i < FLASH_MEMORY_SIZE_GPS; i++){
                    printf("%02x", GPSPtr);
                    GPSPtr++;
                }
                return true;
            }
            return false;
        case BATTERY:
            if(flash_read(DATA_LOGGING_BUFFER_MEMORY_ADDRESS, FLASH_MEMORY_SIZE_BATTERY, FLASH_ADDRESS_BATTERY)){
                BatteryData *BatteryDataPtr = (BatteryData *) DATA_LOGGING_BUFFER_MEMORY_ADDRESS;
                BatteryData data = *BatteryDataPtr;
                //printf modified to send to serial
                printf("%f, %f, %f, %f, %f, %f", data.voltage, data.current, data.temperature, data.state_of_charge, data.power_usage, data.estimated_life);
                return true;
            }
            return false;
        default:
            return false;
        }
}