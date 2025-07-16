#include "obc_interface.h"
#include "camera.h"
#include "main.h"

// TODO: Move flash addresses to main.h and include each used address

// I2C Addresses (left shift for HAL)
#define ALTI_ADDR (0x77 << 1) // 0x77 if CSB pin is pulled low, 0x76 if CSB is pulled high
#define ACCEL_ADDR (0x1E << 1) // 0x1E if ADDR pin is pulled low, 0x1F if ADDR is pulled high
#define GYRO_ADDR (0x68 << 1) // 0x68 if SDO pin is pulled low, 0x69 if SDO is pulled high
#define TEMP_ADDR (0x40 << 1) // 0x40 if ADD0 pin is pulled low, 0x41 if ADD0 is pulled high

// I2C transmit timeout (ms)
#define I2C_Timeout 1000

#define FLASH_SAVE_ADDRESS  ((uint32_t)0x081E0000) // Example sector 7 start (adjust based on your chip)
#define FLASH_SENSOR_ADDRESS FLASH_SECTOR_0 // alter to correct section
#define FLASH_MAGIC         ((uint32_t)0xDEADBEEF)

// Battery
extern ADC_HandleTypeDef hadc_voltage; // ADC handler for voltage
extern ADC_HandleTypeDef hadc_current; // ADC handler for current
extern ADC_HandleTypeDef hadc_temperature; // ADC handler for temperature --battery

// Sensors
extern ADC_HandleTypeDef TemperatureSensor; // ADC handler for temperature --sensors
extern ADC_HandleTypeDef PressureSensor; // ADC handler for pressure --sensors

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

float calculate_energy_usage(float power, float dt) {
    static float total_energy = 0;
    total_energy += (power * dt) / 3600.0f;
    return total_energy;
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
    data.total_energy_used = calculate_energy_usage(data.power_usage, dt);
    data.estimated_life = estimate_battery_life(data.state_of_charge, data.power_usage);
    data.magic = FLASH_MAGIC;
    return data;
}

void save_battery_data_to_flash(BatteryData *data) {
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
	    uint32_t numWords = sizeof(BatteryData) / 8;

	    for (uint32_t i = 0; i < numWords; i++) {
	        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, FLASH_SAVE_ADDRESS + (i * 8), src[i]) != HAL_OK) {
	            // Handle write error
	            HAL_FLASH_Lock();
	            return;
	        }
	    }

	    HAL_FLASH_Lock();
}

void load_battery_data_from_flash() {
    BatteryData *flash_data = (BatteryData *)FLASH_SAVE_ADDRESS;
    if (flash_data->magic == FLASH_MAGIC) {
        memcpy(&battery_backup, flash_data, sizeof(BatteryData));
    } else {
        memset(&battery_backup, 0, sizeof(BatteryData));
    }
}

//////////////////sensors functions start

void init_sensors() {
    HAL_ADC_Start(&TemperatureSensor); 
    Hal_ADC_Start(&PressureSensor);
}

void save_sensor_data_to_flash(SensorsData *data){ // write to flash wrapper
    HAL_FLASH_Unlock();

    // 1. Setup flash erase configuration
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;       // Erase by sector
    erase.Sector = FLASH_SENSOR_ADDRESS;                   // Make sure this is correct for your chip!
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

void load_sensor_data_from_flash(){ // retrieves flash data and uses a pointer to write it to the sensor_backup struct
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

// Accelerometer (I2C)
float read_acceleration_x1(){
    return 41;
}
float read_acceleration_x2(){
    return 42;
}
float read_acceleration_x3(){
    return 43;
}

//// Altimeter (I2C)
// TODO: error handling
// To be called by HL
void altimeter_init(){
	altimeter_read_calibration();
}

float altimeter_read(){
	uint32_t alti_adc_pres = 0;
	uint32_t alti_adc_temp = 0;
	altimeter_convert(&alti_adc_pres, &alti_adc_temp);
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

void altimeter_reset(){
	uint8_t command = 0b00011110; // Reset command for altimeter
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, command, 1, I2C_Timeout);
}

// To be used by LL
void altimeter_read_ADC(uint8_t* adc_pres_val, uint8_t* adc_temp_val){
	// Read pressure ADC value
	uint8_t command = 0b01001000; // Initiate pressure conversion command
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	command = 0b00000000; // Read sequence
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, adc_pres_val, 3, I2C_Timeout);
	// Read temperature ADC value
	uint8_t command = 0b01011000; // Initiate temperature conversion command
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	command = 0b00000000; // Read sequence
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, adc_temp_val, 3, I2C_Timeout);
}

void altimeter_read_calibration(){
	uint8_t command = 0b10100010; // Read coefficient 1
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, &alti_calib[0], 2, I2C_Timeout);
	uint8_t command = 0b10100100; // Read coefficient 2
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, &alti_calib[1], 2, I2C_Timeout);
	uint8_t command = 0b10100110; // Read coefficient 3
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, &alti_calib[2], 2, I2C_Timeout);
	uint8_t command = 0b10101000; // Read coefficient 4
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, &alti_calib[3], 2, I2C_Timeout);
	uint8_t command = 0b10101010; // Read coefficient 5
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, &alti_calib[4], 2, I2C_Timeout);
	uint8_t command = 0b10101100; // Read coefficient 6
	HAL_I2C_Master_Transmit(&hi2c4, ALTI_ADDR, &command, 1, I2C_Timeout);
	HAL_I2C_Master_Receive(&hi2c4, ALTI_ADDR, &alti_calib[5], 2, I2C_Timeout);
}

// Temperature (I2C)
//

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
FRESULT store_data(uint8_t* data, uint8_t data_size, uint8_t type){
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
