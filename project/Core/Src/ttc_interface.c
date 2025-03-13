#include "ttc_interface.h"
//===============================================================================
//============================**TRANSMIT**===============================
//Defines for Transmission
#define TRANS_TIMEOUT 1 //maximum timeout for transmission
#define MAX_ATTEMPTS 5 //maximum number of attempts
#define PACKET_SIZE 8 //maximum data chunk transmitted in one SPI transmission

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


    while((ack ==0) && (timeout_count< MAX_ATTEMPTS)){
      //Drive chip select low (acivate) before transmission
      CC12_Select();

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
   CC12_Deselect();
}


//*****************SPI Read Write Helpers CC12******************
//enable SPI (make active by setting low
void CC12_Select(){
      HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_RESET);
}

//disable SPI by driving high (do this at end of transmission)
void CC12_Deselect(){
      HAL_GPIO_WritePin(CC12_CSn_GPIO, CC12_CSn_PIN, GPIO_PIN_SET);
}
//*********************Helpers for TX communication*************
//Used to set a single register (configuration)
void CC12_Set_Register(uint8_t reg, uint8_t value){
  CC12_Select();
  CC12_SPI_Transfer
}
