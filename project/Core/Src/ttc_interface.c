#include "ttc_interface.h"
#include "main.h"

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








void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if (GPIO_Pin==Transciever_exti_Pin){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(myBinarySem01Handle, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}



}



bool ReadRegisterBurst(){
	uint8_t numcmd[2], len[2];
	FIFOsize rx_buf; // Max RX FIFO size

	uint8_t cmd = 0x3F | 0xC0;  // 0x3F: Location of RXFIFO; 0xC0: READ Burst command

	// Step 1: Read NUM_RXBYTES register
	numcmd[0] = 0xD7 | 0x80;  // Read command for 0xD7
	numcmd[1] = 0x00;         // Dummy

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);   //CS low
	HAL_SPI_TransmitReceive(&hspi2, numcmd, len, 2, HAL_MAX_DELAY);  // recieve the number of bytes to read
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);   // CS high

	uint8_t bytes_to_read = len[1] & 0x7F; // mask overflow bit

	if (bytes_to_read > 0 && bytes_to_read <= 128) {
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	    HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);   // transmit the adress and dummy data
	    HAL_SPI_Receive(&hspi2, rx_buf, bytes_to_read, HAL_MAX_DELAY); // recieve the data into a buffer
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	}
	osStatus_t status = osMessageQueuePut(receivequeueHandle, &rx_buf, 0, 0);   //Write to queue
	if (status != osOK) {
	    return false;
	}
	return true;

}
