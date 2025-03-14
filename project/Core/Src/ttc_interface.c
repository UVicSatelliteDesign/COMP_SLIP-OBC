#include "ttc_interface.h"
#include "cc120x_spi.h"
#include "cc120x_spi.c"

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
HAL_StatusTypeDef status;

//Acknowledgement Response
uint8_t ack = 0; 
//===============================================================================
typedef struct {
  uint8_t *data; //pointer to the dynamically allocated data buffer
  uint8_t length; //length of the data in bytes
  uint8_t retries; //number of retransmissions for this packet
  bool ack_req; //if this data type requires an acknowledgement, set by data packager
}TxPayload_t;


//===============================================================================
/*
 *@brief task responsible for handling SPI transmission
* Task waits for transmission data from the RTOS queue
* Queue holds pointers to data structure that is dynamically allocated before queuing
* This task is responsible for freeing the memory it receives ownership of after transmission
* Task is consumer
*/
void vTransmitTask(void *pvParameters){
  TxPayload_t *txBuffer; // pointer who holds received queue data
  rfStatus_t status;
  
  while(1){
    //wait for data to be placed on queue
    if(xQueueReceive(txQueue, &txBuffer, portMAX_DELAY) == pdPASS){
      status = transmit(txBuffer)
      handle_tx_memory(txBuffer);
      }
  }
}

//transmits the data and sends specific error codes to allow for better handling/more customizable behaviot
tx_status_t transmit(TxPayload_t *txBuffer){
  rfStatus_t status;

  if(txBuffer == NULL || txBuffer->data == NULL){
    return TX_ERROR_NULL_BUFFER;
  }

  //retry transimission up to txBuffer-> retries
  for(uint8_t attempt = 0; attempt <= txBuffer->retries; attempt++){
    //check for space on the buffer
    if((cc120xGetTxStatus() & 0xF0) == 0xF0){
      
          // wait for FIFO to clear or handle a timeout        
          //Handle timeout: reset FIFO or return error 
    }
    //write the length first
    status = cc120xSpiWriteTxFifo(&txBuffer->length, 1);//the size of the packet should always be just one byte
      if (status == 0xF0) // FIFO Full
      {
          cc120xSpiCmdStrobe(CC120X_SFTX); // Flush TX FIFO
          continue; // Retry
      }
      else if (status == 0xFF) // SPI Failure
      {
          return TX_ERROR_SPI_FAILURE;
      }
    //write data payload
    status = cc120xSpiWriteTxFifo(txBuffer->data, txBuffer->length);//
      if (status == 0xF0) //FIFO Full
      {
          cc120xSpiCmdStrobe(CC120X_SFTX); // Flush TX FIFO
          continue; // Retry
      }

      //check for SPI Failure
      if (status == 0xFF){
          
          continue; // Retry
      }

    // trigger transmission: send TX Strobe command
    status = cc120xSpiCmdStrobe(CC120X_STX);
    // wait for transmission to complete
    while((cc120xGetTxStatus() & 0x70)==0x20){
      //measure time and timeout
    }
}

//free memory
void handle_tx_memory(TxPayload_t *txBuffer){
  if(txBuffer != NULL){
    free(txBuffer->data);
    free(txBuffer);
  }
}
//get the error message type
type get_tx_error(something){
//complete implementation
}
//transmit message function


