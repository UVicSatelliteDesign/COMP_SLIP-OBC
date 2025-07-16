#include "ttc_interface.h"
#include "main.h"

//===============================================================================
//============================**TRANSMIT**===============================
// Defines for Transmission
#define TRANS_TIMEOUT 1		 // maximum timeout for transmission
#define MAX_ATTEMPTS 5		 // maximum number of attempts
#define MAX_PAYLOAD_SIZE 128 // The maximum allowable buffer size for the CC12x

// CC12x ports
#define CC12_CSn_GPIO GPIOx	  // Chip select port
#define CC12_SPI_GPIO GPIOx	  // SPI port
#define CC12_Event_GPIO GPIOx // GPIO place holder for interrupts (if used), must have EXTI

// CC12x pins
#define CC12_CSn_PIN GPIO_PIN_x	  // Chip select pin place holder
#define CC12_SI_PIN GPIO_PIN_x	  // SPI MOSI compatible pin place holder
#define CC12_SO_PIN GPIO_PIN_x	  // SPI MISO compatible pin place holder
#define CC12_SCLK_PIN GPIO_PIN_x  // Serial clock placeholder for the tranciever
#define CC12_Event_PIN GPIO_PIN_x // GPIO place holder for interrupts (if used), must have EXTI

// CC12x FIFO Transmit Registers
#define CC12_BURST_TRANS 0x40 // burst transmission
#define CC12_TX_FIFO 0x3F	  // Tx FIFO register
#define CC12_TRANS_START 0x35 // starts transmission

#define CC12_NUM_TXBYTES 0xD6  // Num Tx bytes
#define CC12_EXTENDED_REG 0x27 // Extended register space address
#define CC12_WRITE_BYTE 0x00   // Write single byte command

// Acknowledgement Response
uint8_t ack = 0;
//===============================================================================

void transmit()
{
	// Write packet length to NUM_TXBYTES
	CC12_SendCommand(CC12_EXTENDED_REG | CC12_WRITE_BYTE);
	CC12_SendCommand(CC12_NUM_TXBYTES);
	CC12_SendCommand(packet_data_length);
	// send command to transmit the buffer contents
	CC12_SendCommand(CC12_TRANS_START);
	// Write packet data to transmit buffer
	writeToTransmitBuffer(packet_data_buffer, packet_data_length);
}

void writeToTransmitBuffer(uint8_t *data, uint16 length)
{
	// pull cs low to start spi communication
	HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_RESET);

	// wait for MISO to go low
	while (HAL_GPIO_ReadPin(CC12_SPI_GPIO, CC12_SO_PIN))
		;

	// send the burst command
	CC12_SendCommand(CC12_TX_FIFO | CC12_BURST_TRANS);

	// transmit the data packet
	for (int i = 0; i < length; i++)
	{
		HAL_SPI_Transmit(&hspi1, *data, length, HAL_MAX_DELAY);
		while ((STATUS_REGISTER & 0x02) == 0)
			;	// wait for transmission to complete
		data++; // go to next
	}
	// pull CS high to end SPI communication
	HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_SET);
}

void CC12_SendCommand(uint8_t command)
{
	// pull cs low to start spi communication
	HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_RESET);

	// wait for MISO to go low
	while (HAL_GPIO_ReadPin(CC12_SPI_GPIO, CC12_SO_PIN))
		;

	// transmit command over spi
	HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY);
	while ((STATUS_REGISTER & 0x02) == 0)
		;

	// pull CS high to end spi
	HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_SET);
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
