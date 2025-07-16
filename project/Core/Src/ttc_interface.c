#include "ttc_interface.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

#define MAX_PACKET_SIZE 128
#define HEADER_SIZE 6
#define MAX_PAYLOAD_PER_PACKET (MAX_PACKET_SIZE - HEADER_SIZE)
#define SHARED_MEMORY_ADDRESS ((uint8_t *)0x80000000)

uint8_t packet_data_buffer[MAX_PACKET_SIZE];
uint8_t packet_data_length;

typedef enum
{
	PING = 0b00000000,
	NOMINAL = 0b00000001,
	LOW_POWER = 0b00000010,
	TELEMETRY = 0b00000011,
	CAMERA_1_END = 0b00000100,
	CAMERA_1_MF = 0b00000101,
	CAMERA_2_END = 0b00000110,
	CAMERA_2_MF = 0b00000111,
	REQ_RETRANSMISSION = 0b00001000,
	ERROR_CRC = 0b00001001,
	ERROR_DUP = 0b00001010,
	ERROR_LP = 0b00001011,
	ACK_REC_CAMER = 0b00001100,
	ACK_REC_TELEMETRY = 0b00001101,
	ACK_REC_STATUS = 0b00001110,
	ACK_REC_ERROR = 0b00001111,
} PayloadType;

void writeToDataBuffer(uint8_t *buffer, uint8_t *data, int length)
{
	for (int i = 0; i < length; i++)
	{
		buffer[i] = data[i];
	}
}

void generatepacket(uint8_t type, uint8_t *payload, uint8_t payloadLen)
{
	// Send type
	writeToDataBuffer(&packet_data_buffer[0], &type, 1);

	// Send payload
	writeToDataBuffer(&packet_data_buffer[1], payload, payloadLen);

	// Increase sequence number
	sequenceNum++;

	// Send sequence number (little-endian)
	writeToDataBuffer(&packet_data_buffer[payloadLen + 1], (uint8_t *)&sequenceNum, 2);

	// Set packet length
	packet_data_length = payloadLen + 3;
}

void packageAndSendChunks(uint8_t type, uint8_t *payload, uint16_t fullPayloadLen, uint32_t offset)
{
	if (offset < fullPayloadLen)
	{
		// Determine the size of this chunk's payload
		uint8_t chunkLen = (fullPayloadLen - offset > MAX_PAYLOAD_PER_PACKET)
							   ? MAX_PAYLOAD_PER_PACKET
							   : (fullPayloadLen - offset);

		// Send type
		writeToDataBuffer(&packet_data_buffer[0], &type, 1);

		// Send payload chunk
		writeToDataBuffer(&packet_data_buffer[1], &payload[offset], chunkLen);

		// Send offset (little-endian)
		writeToDataBuffer(&packet_data_buffer[chunkLen + 1], (uint8_t *)&offset, 3);

		// Increase sequence number
		sequenceNum++;

		// Send sequence number (little-endian)
		writeToDataBuffer(&packet_data_buffer[chunkLen + 4], (uint8_t *)&sequenceNum, 2);

		// Set packet length
		packet_data_length = chunkLen + 6;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == Transciever_exti_Pin)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(myBinarySem01Handle, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
