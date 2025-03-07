#include "obc_interface.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings
#define T_DATA 0
#define T_WARNING 1
#define T_ERROR 2

// SD card variables
FRESULT res; // FatFS result code
uint32_t byteswritten; // File write count
uint32_t bytesread; // File read count
//uint8_t wtext[] = "Example text to write"; // File write buffer
uint8_t rtext[_MAX_SS]; // File read buffer

uint16_t image_count = 0; // Keep track of images stored for file names

// Mount SD card
void mount_SD(){
	res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
	if (res != FR_OK){
		// TODO: implement error handling
	    Error_Handler();
	}
}

// Format SD card
void format_SD(){
	// TODO: check if SD card is already formatted
	res = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, rtext, sizeof(rtext));
	if (res != FR_OK){
		// TODO: implement error handling
		Error_Handler();
	}
}

// Create folders
void setup_SD(){
	// Could change this layout later
	res = f_mkdir("UVR-SLIP");
	if (res != FR_OK){
		// TODO: implement error handling
		Error_Handler();
	}
	res = f_mkdir("UVR-SLIP/Images");
	if (res != FR_OK){
		// TODO: implement error handling
		Error_Handler();
	}
}

// Store telemetry/errors/etc on SD card
void store_data(uint8_t data[MAX_DATA_SIZE], int type){
	res = f_open(&SDFile, "UVR-SLIP/telemetry.txt", FA_OPEN_APPEND | FA_WRITE);
	if (res != FR_OK){
		// TODO: implement error handling
		Error_Handler();
	}
	// This is probably not the most efficient way to do this
	if (type == T_DATA){
		res = f_write(&SDFile, "DATA: ", strlen((char *)"DATA: "), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
		res = f_write(&SDFile, data, strlen((char *)data), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
		res = f_write(&SDFile, "\n", strlen((char *)"\n"), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
	} else if (type == T_WARNING){
		res = f_write(&SDFile, "WARNING: ", strlen((char *)"WARNING: "), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
		res = f_write(&SDFile, data, strlen((char *)data), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
		res = f_write(&SDFile, "\n", strlen((char *)"\n"), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
	} else if (type == T_ERROR){
		res = f_write(&SDFile, "ERROR: ", strlen((char *)"ERROR: "), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
		res = f_write(&SDFile, data, strlen((char *)data), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
		res = f_write(&SDFile, "\n", strlen((char *)"\n"), (void *)&byteswritten);
		if((byteswritten == 0) || (res != FR_OK)){
			// TODO: implement error handling
			Error_Handler();
		}
	}
	f_close(&SDFile);
}

// Store images on SD card
void store_image(/* TODO: arguments (JPEG data) */){
	uint8_t size = strlen("UVR-SLIP/Images/image.txt") + 10;
	char path[size];
	snprintf(path, size, "UVR-SLIP/Images/image%04d.txt", image_count);
	res = f_open(&SDFile, path, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK){
		// TODO: implement error handling
		Error_Handler();
	}

	// TODO: store JPEG data in opened file

	f_close(&SDFile);
	image_count++;
}

// Unmount SD card
void unmount_SD(){
	f_mount(&SDFatFS, (TCHAR const*)NULL, 0);
}
