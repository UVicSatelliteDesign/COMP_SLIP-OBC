#ifndef SRC_OBC_INTERFACE_H
#define SRC_OBC_INTERFACE_H

#include "main.h"
#include "fatfs.h"

// For type argument of store_data
enum Type {
	T_DATA,
	T_WARNING,
	T_ERROR
};

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings

// Battery data structure
typedef struct {
    float voltage;
    float current;
    float temperature;
    float state_of_charge;
    float power_usage;
    float estimated_life;
} BatteryData;

// BMS prototypes
void init_bms();
float read_battery_voltage();
float read_battery_current();
float read_battery_temperature();
float calculate_state_of_charge(float current, float dt);
float calculate_state_of_charge(float current, float dt);
float calculate_power_usage(float voltage, float current);
float calculate_energy_usage(float power, float dt);
float estimate_battery_life(float state_of_charge, float avg_power_draw);
BatteryData get_battery_data(float dt);
void save_battery_data_to_flash(BatteryData *data);
void load_battery_data_from_flash();

////////////sensors start
typedef struct { 		// struct containing sensor data values
    float temperature_obc;      // temperature, celsius
    float temperature_ttc;
    float temperature_bms;
    float gyroscope_axis_1;     // gyroscope x axis
    float gyroscope_axis_2;     // gyroscope y axis
    float gyroscope_axis_3;     // gyroscope z axis
    float acceleration_x;  // accelerometer x axis
    float acceleration_y;  // accelerometer y axis
    float acceleration_z;  // accelerometer z axis
    float altitude;				// altimeter altitude
} SensorsData;

void init_sensors(); // initialises pins
void save_sensor_data_to_flash(SensorsData *data); // flash storage macro
void load_sensor_data_from_flash(); // flash retrieval macro

/*
read_sensors:
    macro polling sensor data to a SensorsData struct
Parameters:
    null
Return:
    returns a SensorsData struct  
*/
SensorsData read_sensors(); 

void init_bms();
float read_battery_voltage();
float read_battery_current();
float read_battery_temperature();
float calculate_state_of_charge(float current, float dt);
float calculate_power_usage(float voltage, float current);
float estimate_battery_life(float state_of_charge, float avg_power_draw);
BatteryData get_battery_data(float dt);
void save_battery_data_to_flash(BatteryData *data);
void load_battery_data_from_flash();


//// Temperature sensors
float read_OBC_temperature(); // poll temperature sensor
float read_TTC_temperature_1();
float read_TTC_temperature_2();
float read_BMS_temperature_1();
float read_BMS_temperature_2();

//// Gyroscope
float read_gyroscope_x1(); // poll gyroscope
float read_gyroscope_x2();
float read_gyroscope_x3();

//// Accelerometer
// For use by HL
/**
 * @brief Sets accelerometer range to +/- 64g
 * @return 0 if success, 1 if not
 */
uint8_t accelerometer_init();

/**
 * @brief Reads x-axis acceleration
 * @return x-axis acceleration in g's
 */
float read_acceleration_x();

/**
 * @brief Reads y-axis acceleration
 * @return y-axis acceleration in g's
 */
float read_acceleration_y();

/**
 * @brief Reads z-axis acceleration
 * @return z-axis acceleration in g's
 */
float read_acceleration_z();

//// Altimeter
// For use by HL
/**
 * @brief Reads altimeter calibration constants
 * @return 0 if success, 1 if not
 */
uint8_t altimeter_init();

/**
 * @brief Calculates altitude from pressure
 * @return Altitude in meters
 */
float altimeter_read();

/**
 * @brief Resets altimeter (may not be necessary)
 * @return 0 if success, 1 if not
 */
uint8_t altimeter_reset();

// For use by LL
/**
 * @brief Reads pressure from altimeter
 * @return ADC value for pressure
 */
uint32_t altimeter_read_pressure();

/**
 * @brief Reads temperature value from altimeter
 * @return ADC value for temperature
 */
uint32_t altimeter_read_temperature();

/////////////////sensors end

// Memory prototypes
/**
 * @brief Mounts SD card
 * @return FR_OK if success
 */
FRESULT mount_SD();

/**
 * @brief Formats SD card with FatFS file system
 * @return FR_OK if success
 */
FRESULT format_SD();

/**
 * @brief Creates directories on SD card
 * @return FR_OK if success
 */
FRESULT setup_SD();

/**
 * @brief Stores telemetry, errors, etc (text) on SD card
 * @param data The data to be written
 * @param data_size The size of the data (e.g. string length)
 * @param type Type of data (T_DATA, T_WARNING, or T_ERROR)
 * @return FR_OK if success
 */
FRESULT store_data(uint8_t* data, uint8_t data_size, enum Type type);

/**
 * @brief Stores images in JPEG format on SD card
 * @param data The image data to be written
 * @param data_size The size of the data
 * @return FR_OK if success
 */
FRESULT store_image(uint8_t* data, uint8_t data_size);

/**
 * @brief Unmounts SD card
 * @return FR_OK if success
 */
FRESULT unmount_SD();

#endif /* SRC_OBC_INTERFACE_H_ */
