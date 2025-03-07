#include "ttc.h"

#define TRANS_TIMEOUT 1
#define MAX_ATTEMPTS 5
#define PACKET_SIZE 8 //Maximum data chunk transmitted in one SPI transmission

//Definitions for SPI port and pins
#define Transcievr_CS GPIO_PIN_x //Chip select pin place holder
#define Transcievr_GPIO_Port GPIOx //Transciever port
HAL_StatusTypeDef status;
uint8_t ack = 0; //acknowledgement response

//Handle transmission
void transmit(uint16_t *data, uint16_t data_size){
  uint16_t byte_offset = 0; //offset to track number of bytes sent
  uint16_t data_chunk_size = 0; //the current packet size
  uint8_t timeout_count = 0; //keep track of the number of timeouts
  
  while(byte_offset < sizeof(data_size)){
    data_chunk_size = (data_size - byte_offset) > PACKET_SIZE ? PACKET_SIZE : (data_size - byte_offset);
    timout_count = 0; 
    while((ack ==0) && (timeout_count< MAX_ATTEMPTS)){
      //Drive chip select low (acivate)
      HAL_GPIO_WritePin(Transcievr_GPIO_Port, Transcievr_CS, GPIO_PIN_RESET);

      // get the status of the HAL transmission
      status = HAL_SPI_TRANSMIT(hspi, &data[byte_offset], data_chunk_size, TRANS_TIMEOUT);

      if(status == HAL_OK){
        //data trasmitted successfully 
      }else{
        timeout_count++;
      }
    }
    // recalculate the new data chunk to transmit
    byte_offset += data_chunk_size;
  }
  //Drive high to deactivate
  HAL_GPIO_WritePin(Transcievr_GPIO_Port, Transcievr_CS, GPIO_PIN_SET);
}
