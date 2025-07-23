#include "ttc_interface.h"

#include "flash_interface.h"
#include "stm32h7xx_hal.h"

uint8_t gps_rx_buffer[]; // gps reception buffer
//////////////// section variables need definition
uint8_t NMEA_sentence_size; //  the length in bits of the gps data sentence
uint8_t interrupt_timeout_length; //  the time until the CPU unfreezes if data has not yet been received
uint16_t GPS_FLASH_ADDRESS; // to be set before implementation
////////////////
extern UART_HandleTypeDef huart4; // GPS UART handle definition


/*
get_gps:
    polls an NMEA $GPRMC gps sentence from the gps unit and stores this data to flash
Parameters:
    void
Returns:
    returns a true boolean if successful, and a false boolean otherwise

*/
bool get_gps(){ // this is formatted for polling
    HAL_UART_Receive (&huart4, gps_rx_buffer, NMEA_sentence_size, interrupt_timeout_length);
    if(flash_write(gps_rx_buffer, NMEA_sentence_size, FLASH_SECTOR_GPS, 0) == true){
        return true;
    }
    return false;
}

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

#define MAX_PACKET_SIZE 128
#define HEADER_SIZE 6
#define MAX_PAYLOAD_PER_PACKET (MAX_PACKET_SIZE - HEADER_SIZE)
#define SHARED_MEMORY_ADDRESS ((uint8_t *)0x80000000)

//===============================================================================
//============================**TRANSMIT**===============================
// Defines for Transmission
#define TRANS_TIMEOUT 1		 // maximum timeout for transmission
#define MAX_ATTEMPTS 5		 // maximum number of attempts
#define MAX_PAYLOAD_SIZE 128 // The maximum allowable buffer size for the CC12x

// CC12x FIFO Transmit Registers
#define CC12_BURST_TRANS 0x40 // burst transmission
#define CC12_TX_FIFO 0x3F	  // Tx FIFO register
#define CC12_TRANS_START 0x35 // starts transmission

#define CC12_NUM_TXBYTES 0xD6  // Num Tx bytes
#define CC12_EXTENDED_REG 0x27 // Extended register space address
#define CC12_WRITE_BYTE 0x00   // Write single byte command

uint8_t packet_data_buffer[MAX_PACKET_SIZE];
uint8_t packet_data_length;

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

void packageAndSendChunks(int camera, uint8_t *payload, uint16_t fullPayloadLen, int offset)
{
	if (offset < fullPayloadLen)
	{
		// Determine the size of this chunk's payload
		uint8_t chunkLen = (fullPayloadLen - offset > MAX_PAYLOAD_PER_PACKET)
							   ? MAX_PAYLOAD_PER_PACKET
							   : (fullPayloadLen - offset);

		// Send type
		uint8_t type;
		if (camera == 1) {
			if (offset + chunkLen < fullPayloadLen) {
				type = CAMERA_1_MF;
			} else {
				type = CAMERA_1_END;
			}
		} else {
			if (offset + chunkLen < fullPayloadLen) {
				type = CAMERA_2_MF;
			} else {
				type = CAMERA_2_END;
			}
		}
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

sequenceNum = 0;

void transmit()
{
	// Write packet length to NUM_TXBYTES
	uint8_t command[3] = {CC12_EXTENDED_REG | CC12_WRITE_BYTE, CC12_NUM_TXBYTES, packet_data_length};
	CC12_SendCommand(command, 3);
	// send command to transmit the buffer contents
	uint8_t cmd = CC12_TRANS_START;
	CC12_SendCommand(&cmd, 1);
	// Write packet data to transmit buffer
	writeToTransmitBuffer(packet_data_buffer, packet_data_length);
}

void writeToTransmitBuffer(uint8_t *data, uint16_t length)
{
	// pull CS low to start SPI communication
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // CS low

	// send the burst command
	int cmd = CC12_TX_FIFO | CC12_BURST_TRANS;
	CC12_SendCommand(&cmd, 1);

	// transmit the data packet
	for (int i = 0; i < length; i++)
	{
		HAL_SPI_Transmit(&hspi1, *data, length, HAL_MAX_DELAY);
		data++; // go to next
	}
	// pull CS high to end SPI communication
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // CS high
}

void CC12_SendCommand(uint8_t *command, int length)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // CS low

	// transmit command over spi
	HAL_SPI_Transmit(&hspi1, command, length, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // CS high
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

/**
 * @brief Reads data from the RX FIFO of the CC1201 using SPI burst read mode.
 *
 * This function performs a burst read from the RX FIFO of the CC1201 transceiver.
 * It first reads the NUM_RXBYTES register to determine how many bytes are available
 * in the RX FIFO. If bytes are available, it performs a burst read using the SPI interface.
 * The received data is then placed into a message queue for further processing.
 *
 * Steps:
 * 1. Read the NUM_RXBYTES register (0xD7) to get the number of bytes in the RX FIFO.
 * 2. If bytes are available (1 to 128), send a burst read command (0xFF) to read data.
 * 3. Store the received data into a queue (e.g., RTOS message queue) for downstream use.
 * *
 * @return true if the operation completes successfully and the data is placed into the queue.
 * @return false if writing to the queue fails.
 */

bool ReadRegisterBurst()
{
	uint8_t numcmd[3], len[2];
	FIFOsize rx_buf; // Max RX FIFO size

	uint8_t cmd = 0x3F | 0xC0; // 0x3F: Location of RXFIFO; 0xC0: READ Burst command

	// Step 1: Read NUM_RXBYTES register
	numcmd[0] = 0x2F | 0x80; // Read extended memory space
	numcmd[1] = 0xD7;		 // Command
	numcmd[2] = 0x00;		 // Dummy

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);			// CS low
	HAL_SPI_TransmitReceive(&hspi2, numcmd, len, 3, HAL_MAX_DELAY); // recieve the number of bytes to read
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);			// CS high

	uint8_t bytes_to_read = len[1] & 0x7F; // mask overflow bit

	if (bytes_to_read > 0 && bytes_to_read <= 128)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);			   // transmit the adress and dummy data
		HAL_SPI_Receive(&hspi2, rx_buf, bytes_to_read, HAL_MAX_DELAY); // recieve the data into a buffer
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	}
	osStatus_t status = osMessageQueuePut(receivequeueHandle, &rx_buf, 0, 0); // Write to queue
	if (status != osOK)
	{
		return false;
	}
	return true;
}

