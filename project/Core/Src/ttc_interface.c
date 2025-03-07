#include "ttc_interface.h"
//===============================================================================
//============================**TRANSMIT**===============================
//Defines for Transmission
#define TRANS_TIMEOUT 1 //maximum timeout for transmission
#define MAX_ATTEMPTS 5 //maximum number of attempts
#define PACKET_SIZE 8 //maximum data chunk transmitted in one SPI transmission

//Port and Pin Definitions
#define Transciever_CS GPIO_PIN_x //Chip select pin place holder
#define Transciever_GPIO_Port GPIOx //Transciever port placeholder

HAL_StatusTypeDef status;

//Acknowledgement Response
uint8_t ack = 0; 
//===============================================================================
//===============================================================================

void transmit(uint16_t *data, uint16_t data_size){
  //Variables to hold 
  uint16_t byte_offset = 0; //offset to track number of bytes sent
  uint16_t data_chunk_size = 0; //the current packet size
  uint8_t timeout_count = 0; //keep track of the number of timeouts

  //exit if provided with invalid parameters
  if((data == null || data_size==0)){
    return HAL_ERROR; //this can be changed
  }

  //first while loop 
  while(byte_offset < sizeof(data_size)){
    data_chunk_size = (data_size - byte_offset) > PACKET_SIZE ? PACKET_SIZE : (data_size - byte_offset); 
    timeout_count = 0; 
    while((ack ==0) && (timeout_count< MAX_ATTEMPTS)){
      //Drive chip select low (acivate) before transmission
      HAL_GPIO_WritePin(Transciever_GPIO_Port, Transciever_CS, GPIO_PIN_RESET);

      //Get the status of the HAL transmission
      status = HAL_SPI_TRANSMIT(hspi, &data[byte_offset], data_chunk_size, TRANS_TIMEOUT);

      if(status == HAL_OK){
        ack = 1; // once we have sent all send this acknowledge 
      }else{
        timeout_count++; //increment the number of timeouts if transmission failed
      }
    }
    //recalculate the new data chunk to transmit
    byte_offset += data_chunk_size;
  }
  //Drive high to deactivate
  HAL_GPIO_WritePin(Transcievr_GPIO_Port, Transcievr_CS, GPIO_PIN_SET);
}
