#ifndef TTC_INTERFACE_H
#define TTC_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "cmsis_os2.h"
#include <semphr.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"

typedef uint8_t FIFOsize[128]; // new typedef used for defining FIFO size as mentioned in the CC1201 datasheet

typedef enum
{
	PING = 0b00000000,
	REQ_NOMINAL = 0b00000001,
	REQ_LOW_POWER = 0b00000010,
	TELEMETRY = 0b00000011,
	CAMERA_1_END = 0b00000100,
	CAMERA_1_MF = 0b00000101,
	CAMERA_2_END = 0b00000110,
	CAMERA_2_MF = 0b00000111,
	REQ_RETRANSMISSION = 0b00001000,
	ERROR_PER = 0b00001001,
	ERROR_DUP = 0b00001010,
	ERROR_LP = 0b00001011,
	ACK_REC_CAMER = 0b00001100,
	ACK_REC_TELEMETRY = 0b00001101,
	ACK_REC_STATUS = 0b00001110,
	ACK_REC_ERROR = 0b00001111,
} PayloadType;

const MAX_TRANS_ATTEMPTS = 5;

extern osMessageQueueId_t receivequeueHandle; // the message queue used for receiving info from the trasnciever
extern osSemaphoreId_t myBinarySem01Handle;   // used to protect the dirty bit. it is a sempahore
void writeToDataBuffer(uint8_t *buffer, uint8_t *data, int length);
void generatepacket(uint8_t type, uint8_t *payload, uint8_t payloadLen);
void packageAndSendChunks(int camera, int flash_sector, uint16_t fullPayloadLen, int offset);





/*
get_gps:
    polls an NMEA $GPRMC gps sentence from the gps unit and stores this data to flash
Parameters:
    void
Returns:
    returns a true boolean if successful, and a false boolean otherwise

*/
//////////////// section variables need definition
uint8_t NMEA_sentence_size; //  the length in bytes of the gps data sentence
uint8_t interrupt_timeout_length; //  the time until the CPU unfreezes if data has not yet been received
uint16_t GPS_FLASH_ADDRESS; // to be set before implementation
////////////////
extern UART_HandleTypeDef huart4; // GPS UART handle definition
uint8_t get_gps();


void transmit();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


bool ReadRegisterBurst();
uint16_t sequenceNum; // Current sequence number

#endif
