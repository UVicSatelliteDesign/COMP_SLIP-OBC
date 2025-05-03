#include "obc_interface.h"
#include "stm32h7xx_hal.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#define FLASH_SAVE_ADDRESS  ((uint32_t)0x081E0000) // Example sector 7 start (adjust based on your chip)
#define FLASH_MAGIC         ((uint32_t)0xDEADBEEF)

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

// SD card variables
FRESULT res; // FatFS result code
uint32_t byteswritten; // File write count
uint32_t bytesread; // File read count
//uint8_t wtext[] = "Example text to write"; // File write buffer
uint8_t rtext[_MAX_SS]; // File read buffer

uint16_t image_count = 0; // Keep track of images stored for file names

// Struct to store battery data
typedef struct {
    float voltage;
    float current;
    float temperature;
    float state_of_charge;
    float power_usage;
    float total_energy_used;
    float estimated_life;
    uint32_t magic;
} BatteryData;

BatteryData battery_backup = {0};

void init_bms() {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_Start(&hadc2);
    HAL_ADC_Start(&hadc3);
}

float read_battery_voltage() {
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    return (raw / 4095.0f) * 3.3f * 11.0f;
}

float read_battery_current() {
    HAL_ADC_PollForConversion(&hadc2, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc2);
    return (raw / 4095.0f) * 3.3f / 0.01f;
}

float read_battery_temperature() {
    HAL_ADC_PollForConversion(&hadc3, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc3);
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

    FLASH_EraseInitTypeDef erase;
    uint32_t pageError;
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = FLASH_SECTOR_7; // Change based on actual layout
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    HAL_FLASHEx_Erase(&erase, &pageError);

    uint32_t *src = (uint32_t *)data;
    for (uint32_t i = 0; i < sizeof(BatteryData) / 4; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_SAVE_ADDRESS + i * 4, src[i]);
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







