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


void init_sensors() {
    HAL_ADC_Start(&TemperatureSensor); 
    Hal_ADC_START(&PressureSensor);
}

SensorsData load_sensor_data_from_flash()(){ // retrieve from flash wrapper
    // void Flash_Read_Data (uint32_t StartPageAddress, uint32_t *RxBuf, uint16_t numberofwords); 
    SensorsData sensor_flash_data = dummy_flash_address;
    return sensor_flash_data;
}

void save_sensor_data_to_flash(SensorsData *data){ // write to flash wrapper
    // uint32_t Flash_Write_Data (uint32_t StartPageAddress, uint32_t *Data, uint16_t numberofwords);
    dummy_flash_address = data;
}

float read_temperature(){ // temperature hardware wrapper
    //dummy value degree celsius
    return 10;
}

float read_pressure(){ // pressure hardware wrapper
    //dummy value atmospheres
    return 20;
}

//writes current sensor values to flash/global struct and returns struct with final values
SensorsData read_sensors(){ 
    SensorsData data; // initialise empty struct and/or write over flash
    data.temperature = read_temperature(); // storetemperature and pressure to struct
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