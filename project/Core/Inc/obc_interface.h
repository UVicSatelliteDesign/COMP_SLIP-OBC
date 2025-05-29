#ifndef OBC_INTERFACE_H
#define OBC_INTERFACE_H

#define T_DATA 0
#define T_WARNING 1
#define T_ERROR 2

#define MAX_DATA_SIZE 100 // For telemetry, errors, warnings
#define MAX_IMAGE_BUFFER_SIZE 100 // TODO: Temp

void store_data(char* message, int status);

void freeImageBuffer();

int capture_snapshot();

void save_image_to_flash();

void Flash_Read_Data();

void store_image();

void read_bms();

void read_sensors();

#endif // OBC_INTERFACE_H
