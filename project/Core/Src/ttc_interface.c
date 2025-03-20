#include "ttc_interface.h"

//===============================================================================
//============================**TRANSMIT**===============================
//Defines for Transmission
#define TRANS_TIMEOUT 1 //maximum timeout for transmission
#define MAX_ATTEMPTS 5 //maximum number of attempts
#define MAX_PAYLOAD_SIZE 256 //The maximum allowable buffer size for the CC12x

//CC12x ports
#define CC12_CSn_GPIO                      GPIOx //Chip select port
#define CC12_SPI_GPIO                      GPIOx //SPI port
#define CC12_Event_GPIO                    GPIOx //GPIO place holder for interrupts (if used), must have EXTI

//CC12x pins
#define CC12_CSn_PIN                      GPIO_PIN_x //Chip select pin place holder
#define CC12_SI_PIN                       GPIO_PIN_x //SPI MOSI compatible pin place holder
#define CC12_SO_PIN                       GPIO_PIN_x //SPI MISO compatible pin place holder
#define CC12_SCLK_PIN                     GPIO_PIN_x //Serial clock placeholder for the tranciever
#define CC12_Event_PIN                    GPIO_PIN_x //GPIO place holder for interrupts (if used), must have EXTI

//CC12x FIFO Transmit Registers
#define CC12_BURST_TRANS                  0x007F //burst transmission
#define CC12_TRANS_START                  0x35 //starts transmission

//Acknowledgement Response
uint8_t ack = 0; 
//===============================================================================

void transmit(){
  //send command to transmit the buffer contents
  CC12_SendCommand(CC12_TRANS_START);
}

void writeToTransmitBuffer(uint8_t *data){
  //pull cs low to start spi communication
  HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_RESET);

  //wait for MISO to go low
  while(HAL_GPIO_ReadPin(CC12_SPI_GPIO,CC12_SO_PIN));

  //send the burst command
  CC12_SendCommand(CC12_BURST_TRANS);

  HAL_SPI_Transmit(&hspi1, data, length, HAL_MAX_DELAY);
  
  //pull CS high to end SPI communication
  HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_SET);
}


void CC12_SendCommand(uint8_t command){
  //pull cs low to start spi communication
  HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_RESET);

  //wait for MISO to go low
  while(HAL_GPIO_ReadPin(CC12_SPI_GPIO,CC12_SO_PIN));

  //transmit command over spi
  HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY);

  //pull CS high to end spi
  HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_SET);
}
