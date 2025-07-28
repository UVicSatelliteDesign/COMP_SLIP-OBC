#include "obc_interface.h"
#include "camera.h"
#include "main.h"
#include "flash_interface.h"

// TODO: Move flash addresses to main.h and include each used address

// I2C Addresses (left shift for HAL)
#define ALTI_ADDR (0x76 << 1) // 0x77 if CSB pin is pulled low, 0x76 if CSB is pulled high
#define ACCEL_ADDR (0x1E << 1) // 0x1E if ADDR pin is pulled low, 0x1F if ADDR is pulled high
#define GYRO_ADDR (0x68 << 1) // 0x68 if SDO pin is pulled low, 0x69 if SDO is pulled high
#define TEMP_ADDR_TTC_1 (0x4A << 1)
#define TEMP_ADDR_TTC_2 (0x4B << 1)
#define TEMP_ADDR_BMS_1 (0x48 << 1)
#define TEMP_ADDR_BMS_2 (0x49 << 1)
#define BMS_ADC_ADDR (0x10 << 1)

// I2C transmit timeout (ms)
#define I2C_Timeout 1000

#define FLASH_SAVE_ADDRESS  ((uint32_t)0x081E0000) // Example sector 7 start (adjust based on your chip)
#define FLASH_SENSOR_ADDRESS FLASH_SECTOR_0 // alter to correct section
#define FLASH_MAGIC         ((uint32_t)0xDEADBEEF)

extern I2C_HandleTypeDef hi2c2; // I2C handler for camera 1, BMS ADC sensors, BMS and TTC temperature sensors
extern I2C_HandleTypeDef hi2c4; // I2C handler for camera 2, accelerometer, altimeter, gyroscope

// Battery
extern ADC_HandleTypeDef hadc_voltage; // ADC handler for voltage
extern ADC_HandleTypeDef hadc_current; // ADC handler for current
extern ADC_HandleTypeDef hadc_temperature; // ADC handler for temperature --battery

// Sensors
extern ADC_HandleTypeDef TemperatureSensor; // ADC handler for temperature --sensors

// Altimeter calibration constants
uint16_t alti_calib[6] = {};

SensorsData sensor_backup = {0}; // Data is written to this by pointer when retrieved from flash memory

// SD card variables
FRESULT res; // FatFS result code
uint32_t byteswritten; // File write count
uint32_t bytesread; // File read count
//uint8_t wtext[] = "Example text to write"; // File write buffer
uint8_t rtext[_MAX_SS]; // File read buffer

uint16_t image_count = 0; // Keep track of images stored for file names

BatteryData battery_backup = {0};

void init_bms() {
    HAL_ADC_Start(&hadc_voltage);
    HAL_ADC_Start(&hadc_current);
    HAL_ADC_Start(&hadc_temperature);
}

float read_battery_voltage() {
    HAL_ADC_PollForConversion(&hadc_voltage, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc_voltage);
    return (raw / 4095.0f) * 3.3f * 11.0f;
}

float read_battery_current() {
    HAL_ADC_PollForConversion(&hadc_current, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc_current);
    return (raw / 4095.0f) * 3.3f / 0.01f;
}

float read_battery_temperature() {
    HAL_ADC_PollForConversion(&hadc_temperature, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc_temperature);
    float resistance = (10000.0f * raw) / (4095.0f - raw);
    return 1.0f / (0.001129148f + (0.000234125f * logf(resistance))) - 273.15f;
}

float calculate_state_of_charge(float current, float dt) {
    static float accumulated_charge = 0;
    float battery_capacity_Ah = 3.0f;
    accumulated_charge += (current * dt) / 3600.0f;
    float soc = (1.0f - (accumulated_charge / battery_capacity_Ah)) * 100.0f;
    return soc > 0 ? soc : 0;
}

float calculate_power_usage(float voltage, float current) {
    return voltage * current;
}

float estimate_battery_life(float state_of_charge, float avg_power_draw) {
    float battery_capacity_Wh = 50.0f;
    float remaining_energy = (state_of_charge / 100.0f) * battery_capacity_Wh;
    return (avg_power_draw > 0) ? (remaining_energy / avg_power_draw) : -1;
}

BatteryData get_battery_data(float dt) {
    BatteryData data;
    data.voltage = read_battery_voltage();
    data.current = read_battery_current();
    data.temperature = read_battery_temperature();
    data.state_of_charge = calculate_state_of_charge(data.current, dt);
    data.power_usage = calculate_power_usage(data.voltage, data.current);
    data.estimated_life = estimate_battery_life(data.state_of_charge, data.power_usage);
    return data;
}

void save_battery_data_to_flash(BatteryData *data) {
    if (!flash_write(data, sizeof(BatteryData), FLASH_SECTOR_BATTERY, BATTERY_DATA_OFFSET)) {
        // TODO: Handle flash write error
    }
}

void load_battery_data_from_flash() {
    if (!flash_read(&battery_backup, sizeof(BatteryData), FLASH_SECTOR_BATTERY, BATTERY_DATA_OFFSET)) {
        memset(&battery_backup, 0, sizeof(BatteryData));
    }
}

//////////////////sensors functions start

void init_sensors() {
    HAL_ADC_Start(&TemperatureSensor); 
    HAL_ADC_Start(&PressureSensor);
}

void save_sensor_data_to_flash(SensorsData *data) {
    if (!flash_write(data, sizeof(SensorsData), FLASH_SECTOR_SENSORS, SENSOR_DATA_OFFSET)) {
        // TODO: Handle flash write error
    }
}

void load_sensor_data_from_flash() {
    if (!flash_read(&sensor_backup, sizeof(SensorsData), FLASH_SECTOR_SENSORS, SENSOR_DATA_OFFSET)) {
        memset(&sensor_backup, 0, sizeof(SensorsData));
    }
}

//// Temperature sensors
float read_TTC_temperature_1() {
    uint8_t raw[2] = {0};
    int16_t temp_raw = 0;

    if (HAL_I2C_Mem_Read(&hi2c2, TEMP_ADDR_TTC_1, 0x00, I2C_MEMADD_SIZE_8BIT, raw, 2, I2C_Timeout) != HAL_OK) {
        return 100000.0f;
    }

    temp_raw = (int16_t)((raw[0] << 4) | raw[1] >> 4);
    return (float)temp_raw +273.15 ; // Convert to Kelvin
}

float read_TTC_temperature_2() {
    uint8_t raw[2] = {0};
    int16_t temp_raw = 0;

    if (HAL_I2C_Mem_Read(&hi2c2, TEMP_ADDR_TTC_2, 0x00, I2C_MEMADD_SIZE_8BIT, raw, 2, I2C_Timeout) != HAL_OK) {
        return 100000.0f;
    }

    temp_raw = (int16_t)((raw[0] << 4) | raw[1] >> 4);
    return (float)temp_raw +273.15 ; // Convert to Kelvin
}   

float read_BMS_temperature_1() {
    uint8_t raw[2] = {0};
    int16_t temp_raw = 0;

    if (HAL_I2C_Mem_Read(&hi2c2, TEMP_ADDR_BMS_1, 0x00, I2C_MEMADD_SIZE_8BIT, raw, 2, I2C_Timeout) != HAL_OK) {
        return 100000.0f;
    }

    temp_raw = (int16_t)((raw[0] << 4) | raw[1] >> 4);
    return (float)temp_raw +273.15 ; // Convert to Kelvin
}   

float read_BMS_temperature_2() {
    uint8_t raw[2] = {0};
    int16_t temp_raw = 0;

    if (HAL_I2C_Mem_Read(&hi2c2, TEMP_ADDR_BMS_2, 0x00, I2C_MEMADD_SIZE_8BIT, raw, 2, I2C_Timeout) != HAL_OK) {
        return 100000.0f;
    }

    temp_raw = (int16_t)((raw[0] << 4) | raw[1] >> 4);
    return (float)temp_raw +273.15 ; // Convert to Kelvin
}

float read_OBC_temperature(){ // temperature hardware wrapper
    // //dummy value degree celsius
    // return 10;
    HAL_ADC_PollForConversion(&TemperatureSensor, 100);
    uint32_t raw = HAL_ADC_GetValue(&TemperatureSensor);
    return raw;
}

// Gyroscope (I2C)
float read_gyroscope_x1(){
    return 31;
}
float read_gyroscope_x2(){
    return 32;
}
float read_gyroscope_x3(){
    return 33;
}

//// Accelerometer (I2C)
// To be called by HL
uint8_t accelerometer_init(){
	uint8_t cntl1_addr = (0xB1 << 1);
	uint8_t cntl1 = 0;
	// Read CNTL1 register
	if (HAL_I2C_Mem_Read(&hi2c4, ACCEL_ADDR, cntl1_addr, I2C_MEMADD_SIZE_8BIT, &cntl1, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	// Set PC1 to 0 in CNTL1 to allow writing to other settings (bit 7)
	// Set GSEL<1:0> to 11 for +-64g range (bits 3 and 4)
	uint8_t command = 0b00011000 | cntl1; // Data to be written to register (OR with current to not overwrite reserved bits)
	if (HAL_I2C_Mem_Write(&hi2c4, ACCEL_ADDR, cntl1_addr, I2C_MEMADD_SIZE_8BIT, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	return 0;
	// Read CNTL1 register
	if (HAL_I2C_Mem_Read(&hi2c4, ACCEL_ADDR, cntl1_addr, I2C_MEMADD_SIZE_8BIT, &cntl1, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	// Set PC1 to 1 in CNTL1 to enable accelerometer (bit 7)
	command = 0b10000000 | cntl1; // Data to be written to register (OR with current to not overwrite reserved bits)
	if (HAL_I2C_Mem_Write(&hi2c4, ACCEL_ADDR, cntl1_addr, I2C_MEMADD_SIZE_8BIT, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	return 0;
}

float read_acceleration_x(){
	uint8_t x_LSB_addr = (0x08 << 1);
	uint8_t x_MSB_addr = (0x09 << 1);
	uint8_t x_LSB = 0;
	uint8_t x_MSB = 0;
	// Read x acceleration registers (LSB and MSB)
	if (HAL_I2C_Mem_Read(&hi2c4, ACCEL_ADDR, x_LSB_addr, I2C_MEMADD_SIZE_8BIT, &x_LSB, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFFFFFF;
	}
	if (HAL_I2C_Mem_Read(&hi2c4, ACCEL_ADDR, x_MSB_addr, I2C_MEMADD_SIZE_8BIT, &x_MSB, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFFFFFF;
	}
	// Combine LSB and MSB
	int16_t x = (x_MSB << 8) | x_LSB;
	// Return value in g's
    return x/32768.0*64;
}

float read_acceleration_y(){
	uint8_t y_LSB_addr = (0x0A << 1);
	uint8_t y_MSB_addr = (0x0B << 1);
	uint8_t y_LSB = 0;
	uint8_t y_MSB = 0;
	// Read y acceleration registers (LSB and MSB)
	if (HAL_I2C_Mem_Read(&hi2c4, ACCEL_ADDR, y_LSB_addr, I2C_MEMADD_SIZE_8BIT, &y_LSB, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFFFFFF;
	}
	if (HAL_I2C_Mem_Read(&hi2c4, ACCEL_ADDR, y_MSB_addr, I2C_MEMADD_SIZE_8BIT, &y_MSB, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFFFFFF;
	}
	// Combine LSB and MSB
	int16_t y = (y_MSB << 8) | y_LSB;
	// Return value in g's
	return y/32768.0*64;
}

float read_acceleration_z(){
	uint8_t z_LSB_addr = (0x0C << 1);
	uint8_t z_MSB_addr = (0x0D << 1);
	uint8_t z_LSB = 0;
	uint8_t z_MSB = 0;
	// Read z acceleration registers (LSB and MSB)
	if (HAL_I2C_Mem_Read(&hi2c4, ACCEL_ADDR, z_LSB_addr, I2C_MEMADD_SIZE_8BIT, &z_LSB, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFFFFFF;
	}
	if (HAL_I2C_Mem_Read(&hi2c4, ACCEL_ADDR, z_MSB_addr, I2C_MEMADD_SIZE_8BIT, &z_MSB, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFFFFFF;
	}
	// Combine LSB and MSB
	int16_t z = (z_MSB << 8) | z_LSB;
	// Return value in g's
	return z/32768.0*64;
}

//// Altimeter (I2C)
// To be called by HL
uint8_t altimeter_init(){
	uint8_t temp[2] = {};
	uint8_t command = 0b10100010; // Read coefficient 1
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	if (HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, temp, 2, I2C_Timeout) != HAL_OK){
		return 1;
	}
	// TODO: check if byte order is correct
	alti_calib[0] = temp[0] | (temp[1] << 8);
	command = 0b10100100; // Read coefficient 2
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	if (HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, temp, 2, I2C_Timeout) != HAL_OK){
		return 1;
	}
	// TODO: check if byte order is correct
	alti_calib[1] = temp[0] | (temp[1] << 8);
	command = 0b10100110; // Read coefficient 3
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	if (HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, temp, 2, I2C_Timeout) != HAL_OK){
		return 1;
	}
	// TODO: check if byte order is correct
	alti_calib[2] = temp[0] | (temp[1] << 8);
	command = 0b10101000; // Read coefficient 4
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	if (HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, temp, 2, I2C_Timeout) != HAL_OK){
		return 1;
	}
	// TODO: check if byte order is correct
	alti_calib[3] = temp[0] | (temp[1] << 8);
	command = 0b10101010; // Read coefficient 5
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	if (HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, temp, 2, I2C_Timeout) != HAL_OK){
		return 1;
	}
	// TODO: check if byte order is correct
	alti_calib[4] = temp[0] | (temp[1] << 8);
	command = 0b10101100; // Read coefficient 6
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	if (HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, temp, 2, I2C_Timeout) != HAL_OK){
		return 1;
	}
	// TODO: check if byte order is correct
	alti_calib[5] = temp[0] | (temp[1] << 8);
	return 0;
}

float altimeter_read(){
	uint32_t alti_adc_pres = altimeter_read_pressure();
	uint32_t alti_adc_temp = altimeter_read_temperature();
	// Convert ADC value to altitude (magic numbers come from datasheet)
	int32_t dT = alti_adc_temp - alti_calib[4]*256;
	int32_t temperature = 2000 + dT*alti_calib[5]/8388608;
	int64_t offset = alti_calib[1]*65536 + alti_calib[3]*dT/128;
	int64_t sens = alti_calib[0]*32768 + alti_calib[2]*dT/256;
	int32_t pressure = (alti_adc_pres*sens/2097152 - offset)/32768;
	// TODO: implement second order conversion for improved accuracy
	// Calculate altitude from pressure (and temperature?)
	// TODO: make this formula more accurate
	float altitude = 44330*(1-pow(pressure/(float)101320,1/5.255));
	return altitude;
}

uint8_t altimeter_reset(){
	uint8_t command = 0b00011110; // Reset command for altimeter
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 1;
	}
	return 0;
}

// To be used by LL
uint32_t altimeter_read_pressure(){
	uint8_t adc_pres[3] = {};
	// Read pressure ADC value
	uint8_t command = 0b01001000; // Initiate pressure conversion command
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFF;
	}
	command = 0b00000000; // Read sequence
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFF;
	}
	if (HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, adc_pres, 3, I2C_Timeout) != HAL_OK){
		return 0xFFFF;
	}
	// TODO: check if byte order is correct
	return (adc_pres[0] | (adc_pres[1] << 8) | (adc_pres[2] << 16));
}

uint32_t altimeter_read_temperature(){
	uint8_t adc_temp[3] = {};
	// Read temperature ADC value
	uint8_t command = 0b01011000; // Initiate temperature conversion command
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 0xFFF;
	}
	command = 0b00000000; // Read sequence
	if (HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout) != HAL_OK){
		return 0xFFFF;
	}
	if (HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, adc_temp, 3, I2C_Timeout) != HAL_OK){
		return 0xFFFF;
	}
	// TODO: check if byte order is correct
	return (adc_temp[0] | (adc_temp[1] << 8) | (adc_temp[2] << 16));
}

//writes current sensor values to flash/global struct and returns struct with final values
SensorsData read_sensors(){
    SensorsData data; // initialise empty struct and/or write over flash
    data.temperature_obc = read_OBC_temperature(); // store temperature and pressure to struct
    data.temperature_ttc = read_TTC_temperature_1();
    data.temperature_bms = read_BMS_temperature_1();
    data.gyroscope_axis_1 = read_gyroscope_x1();
    data.gyroscope_axis_2 = read_gyroscope_x2();
    data.gyroscope_axis_3 = read_gyroscope_x3();
    data.acceleration_x = read_acceleration_x();
    data.acceleration_y = read_acceleration_y();
    data.acceleration_z = read_acceleration_z();
    data.altitude = altimeter_read();
    save_sensor_data_to_flash(&data);
    return data; // return filled struct
}

/////////////sensors functions end

//// SD card functions
// Mount SD card
FRESULT mount_SD(){
	res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
	// if (res != FR_OK){
	// 	// Error handling
	// }
	return res;
}

// Format SD card (run once)
FRESULT format_SD(){
	// TODO: check if SD card is already formatted
	res = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, rtext, sizeof(rtext));
	// if (res != FR_OK){
	// 	// Error handling
	// }
	return res;
}

// Create folders (run once)
FRESULT setup_SD(){
	// Could change this layout later
	res = f_mkdir("UVR-SLIP");
	if (res != FR_OK){
		// Error handling
		return res;
	}
	res = f_mkdir("UVR-SLIP/Images");
	// if (res != FR_OK){
	// 	// Error handling
	// 	return res;
	// }
	return res;
}

// Store telemetry/errors/etc on SD card
FRESULT store_data(uint8_t* data, uint8_t data_size, enum Type type){
	res = f_open(&SDFile, "UVR-SLIP/telemetry.txt", FA_OPEN_APPEND | FA_WRITE);
	if (res != FR_OK){
        f_close(&SDFile);
		// Error handling
		return res;
	}
	const char* prefix = NULL;
	switch (type) {
		case T_DATA:
			prefix = "DATA: ";
		break;
		case T_WARNING:
			prefix = "WARNING: ";
		break;
		case T_ERROR:
			prefix = "ERROR: ";
		break;
		default:
			prefix = "UNKNOWN: ";
		break;
	}
	res = f_write(&SDFile, prefix, strlen(prefix), (void *)&byteswritten);
	if((byteswritten == 0) || (res != FR_OK)){
        f_close(&SDFile);
		// Error handling
		return res;
	}
	res = f_write(&SDFile, data, data_size, (void *)&byteswritten);
	if((byteswritten == 0) || (res != FR_OK)){
        f_close(&SDFile);
		// Error handling
		return res;
	}
	res = f_write(&SDFile, "\n", strlen((char *)"\n"), (void *)&byteswritten);
	if((byteswritten == 0) || (res != FR_OK)){
        f_close(&SDFile);
		// Error handling
		return res;
	}
	f_close(&SDFile);
	return res;
}

// Store images on SD card
FRESULT store_image(uint8_t* data, uint8_t data_size){
	uint8_t size = strlen("UVR-SLIP/Images/image.jpeg") + 10;
	char path[size];
	snprintf(path, size, "UVR-SLIP/Images/image%04d.jpeg", image_count);
	res = f_open(&SDFile, path, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK){
        f_close(&SDFile);
		// Error handling
		return res;
	}
	res = f_write(&SDFile, data, data_size, (void *)&byteswritten);
	if((byteswritten == 0) || (res != FR_OK)){
        f_close(&SDFile);
		// Error handling
		return res;
	}
	f_close(&SDFile);
	image_count++;
	return res;
}

// Unmount SD card
FRESULT unmount_SD(){
	return f_mount(&SDFatFS, (TCHAR const*)NULL, 0);
}
