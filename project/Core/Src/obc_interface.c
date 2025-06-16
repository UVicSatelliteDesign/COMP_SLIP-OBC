#include "obc_interface.h"
#include "main.h"

// TODO: Move flash addresses to main.h and include each used address

#define FLASH_SAVE_ADDRESS  ((uint32_t)0x081E0000) // Example sector 7 start (adjust based on your chip)
#define FLASH_SENSOR_ADDRESS FLASH_SECTOR_0 // alter to correct section
#define FLASH_MAGIC         ((uint32_t)0xDEADBEEF)

// Battery
extern ADC_HandleTypeDef hadc_voltage; // ADC handler for voltage
extern ADC_HandleTypeDef hadc_current; // ADC handler for current
extern ADC_HandleTypeDef hadc_temperature; // ADC handler for temperature

// Sensors
extern ADC_HandleTypeDef TemperatureSensor; // ADC handler for temperature
extern ADC_HandleTypeDef PressureSensor; // ADC handler for pressure

SensorsData sensor_backup = {0};

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

BatteryData load_battery_data_from_flash() {
    BatteryData *flash_data = (BatteryData *)FLASH_SAVE_ADDRESS;
    if (flash_data->magic == FLASH_MAGIC) {
        memcpy(&battery_backup, flash_data, sizeof(BatteryData));
    } else {
        memset(&battery_backup, 0, sizeof(BatteryData));
    }
    return *flash_data;
}


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
FRESULT store_data(uint8_t data[MAX_DATA_SIZE], uint8_t type){
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
	res = f_write(&SDFile, data, strlen((char *)data), (void *)&byteswritten);
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
FRESULT store_image(uint8_t data[MAX_IMAGE_BUFFER_SIZE]){
	uint8_t size = strlen("UVR-SLIP/Images/image.jpeg") + 10;
	char path[size];
	snprintf(path, size, "UVR-SLIP/Images/image%04d.jpeg", image_count);
	res = f_open(&SDFile, path, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK){
        f_close(&SDFile);
		// Error handling
		return res;
	}
	res = f_write(&SDFile, data, MAX_IMAGE_BUFFER_SIZE, (void *)&byteswritten);
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
    return *flash_data;
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
    return 31.0;
}
float read_gyroscope_x2(){
    return 32.0;
}

float read_gyroscope_x3(){
    return 33.0;
}

float read_acceleration_x1(){
    return 41.0;
}
float read_acceleration_x2(){
    return 42.0;
}
float read_acceleration_x3(){
    return 43.0;
}
