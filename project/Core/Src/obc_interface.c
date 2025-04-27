#include "obc_interface.h"
//////////////
// README
// todo:
//      freeRTOS: task priority, task depth
//      flash: dedicated memory section
//      sensor hardware implementation
/////////////

// xTaskCreate(Sensors_Task, "Sensors", 256, NULL, 2, NULL);

// uint32_t sensor_flash_address 
// SensorsData get_sensor_flash_data(){ // pseudo code
//     // SensorsData sensor_flash_data = readflash(sensor_flash_address);
//     return sensor_flash_data;
// }

// example address definition: uint32_t sensor_flash_address = 0x08004410;
SensorsData dummy_flash_address; // dummy value global variable, mimics storage in flash address - only use in direct flash interaction functions i.e. flash read, write wrappers


SensorsData read_sensor_flash_data(){ // retrieve from flash wrapper
    // void Flash_Read_Data (uint32_t StartPageAddress, uint32_t *RxBuf, uint16_t numberofwords); 
    SensorsData sensor_flash_data = dummy_flash_address;
    return sensor_flash_data;
}

void write_sensor_flash_data(SensorsData data){ // write to flash wrapper
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
// gyroscope hardware wrapper, writes straight to flash
void read_gyro(){      
    SensorsData data = read_sensor_flash_data();                          
    data.gyroscope_axis_1 = read_gyroscope_x1(); // manage hardware side of getting gyro axis values
    data.gyroscope_axis_2 = read_gyroscope_x2();
    data.gyroscope_axis_3 = read_gyroscope_x3();
    write_sensor_flash_data(data);
}

// manage writing accelerometer values to struct
void read_acceleration(){
    SensorsData data = read_sensor_flash_data();
    data.acceleration_axis_1 = read_acceleration_x1();   // manage hardware side of getting accelerometer axis values
    data.acceleration_axis_2 = read_acceleration_x2();
    data.acceleration_axis_3 = read_acceleration_x3();
    write_sensor_flash_data(data);
}

//writes current sensor values to flash/global struct and returns struct with final values
SensorsData read_sensors(){ 
    SensorsData data; // initialise empty struct and/or write over flash
    data.temperature = read_temperature(); // storetemperature and pressure to struct
    data.pressure = read_pressure();
    write_sensor_flash_data(data); // write struct to memory
    read_gyro(); // gyro and acceleration: hardware wrapper writes multi variable sensor returns straight to flash struct 
    read_acceleration();
    data = read_sensor_flash_data(); // retrieve/copy struct from flash
    return data; // return filled struct
}

float get_temperature(){
    float temperature = read_sensor_flash_data().temperature;
    return temperature;
}

float get_pressure(){
    float pressure = read_sensor_flash_data().pressure;
    return pressure;
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