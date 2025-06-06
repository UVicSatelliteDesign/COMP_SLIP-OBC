#include "obc_interface.h"
#include "stm32h7xx_hal.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
//////////////
// README
// todo:
/////////////
#define FLASH_SENSOR_ADDRESS FLASH_SECTOR_0; // alter to correct section
#define FLASH_MAGIC         ((uint32_t)0xDEADBEEF)

extern ADC_HandleTypeDef TemperatureSensor; // ADC handler for temperature
extern ADC_HandleTypeDef PressureSensor; // ADC handler for pressure

SensorsData sensor_backup = {0};


void init_sensors() {
    HAL_ADC_Start(&TemperatureSensor); 
    Hal_ADC_START(&PressureSensor);
}

void save_sensor_data_to_flash(SensorsData *data){ // write to flash wrapper
    HAL_FLASH_Unlock();

    // 1. Setup flash erase configuration
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;       // Erase by sector
    erase.Sector = FLASH_SECTOR_7;                   // Make sure this is correct for your chip!
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;      // 2.7V to 3.6V

    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        // Handle erase error
        HAL_FLASH_Lock();
        return;
    }

    // 2. Write the data in 64-bit chunks
    uint64_t *src = (uint64_t *)data;
    uint32_t numWords = sizeof(SensorsData) / 8;

    for (uint32_t i = 0; i < numWords; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, FLASH_SENSOR_ADDRESS + (i * 8), src[i]) != HAL_OK) {
            // Handle write error
            HAL_FLASH_Lock();
            return;
        }
    }

    HAL_FLASH_Lock();
}

SensorsData load_sensor_data_from_flash(){ // retrieve from flash wrapper
    SensorsData *flash_data = (SensorsData *)FLASH_SENSOR_ADDRESS;
    if (flash_data->magic == FLASH_MAGIC) {
        memcpy(&sensor_backup, flash_data, sizeof(SensorsData));
    } else {
        memset(&sensor_backup, 0, sizeof(SensorsData));
    }
}



float read_temperature(){ // temperature hardware wrapper
    // //dummy value degree celsius
    // return 10;
    HAL_ADC_PollForConversion(&TemperatureSensor, 100);
    uint32_t raw = HAL_ADC_GetValue(&TemperatureSensor);
    return raw;
}

float read_pressure(){ // pressure hardware wrapper
    // //dummy value atmospheres
    // return 20;
    HAL_ADC_PollForConversion(&PressureSensor, 100);
    uint32_t raw = HAL_ADC_GetValue(&PressureSensor);
    return raw;
}

//writes current sensor values to flash/global struct and returns struct with final values
SensorsData read_sensors(){ 
    SensorsData data; // initialise empty struct and/or write over flash
    data.temperature = read_temperature(); // store temperature and pressure to struct
    data.pressure = read_pressure();
    data.gyroscope_axis_1 = read_gyroscope_x1();
    data.gyroscope_axis_2 = read_gyroscope_x2();
    data.gyroscope_axis_3 = read_gyroscope_x3();
    data.acceleration_axis_1 = read_acceleration_x1();
    data.acceleration_axis_2 = read_acceleration_x2();
    data.acceleration_axis_3 = read_acceleration_x3();
    data.magic = FLASH_MAGIC;
    save_sensor_data_to_flash(&data);
    return data; // return filled struct
}


float read_gyroscope_x1(){
    return 31;
}
float read_gyroscope_x2(){
    return 32;
}

float read_gyroscope_x3(){
    return 33;
}

float read_acceleration_x1(){
    return 41;
}
float read_acceleration_x2(){
    return 42;
}
float read_acceleration_x3(){
    return 43;
}