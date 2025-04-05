#include "obc_interface.h"
#include <stdio.h>
#include <string.h>

// SD card variables
FRESULT res; // FatFS result code
uint32_t byteswritten; // File write count
uint32_t bytesread; // File read count
//uint8_t wtext[] = "Example text to write"; // File write buffer
uint8_t rtext[_MAX_SS]; // File read buffer

uint16_t image_count = 0; // Keep track of images stored for file names

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
		// Error handling
		return res;
	}
	res = f_write(&SDFile, data, strlen((char *)data), (void *)&byteswritten);
	if((byteswritten == 0) || (res != FR_OK)){
		// Error handling
		return res;
	}
	res = f_write(&SDFile, "\n", strlen((char *)"\n"), (void *)&byteswritten);
	if((byteswritten == 0) || (res != FR_OK)){
		// Error handling
		return res;
	}
	f_close(&SDFile);
	return res;
}

// Store images on SD card
FRESULT store_image(uint8_t data[IMAGE_BUFFER_SIZE]){
	uint8_t size = strlen("UVR-SLIP/Images/image.jpeg") + 10;
	char path[size];
	snprintf(path, size, "UVR-SLIP/Images/image%04d.jpeg", image_count);
	res = f_open(&SDFile, path, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK){
		// Error handling
		return res;
	}
	res = f_write(&SDFile, data, IMAGE_BUFFER_SIZE, (void *)&byteswritten);
	if((byteswritten == 0) || (res != FR_OK)){
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
