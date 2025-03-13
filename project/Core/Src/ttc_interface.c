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
      //check if the pointer on the queue is valid
      if(txBuffer != NULL && txBuffer->data != NULL){
        // check the TX FIFO space
        status = cc120xGetTxStatus();
        if((status & 0xF0) == 0xF0){
          // wait for FIFO to clear or handle a timeout
          
          //Handle timeout: reset FIFO or return error 
          
          //then transmit
        }
        // write the packet length first: we use variable packet mode
        status = cc120xSpiWriteTxFifo(&txBuffer->length, 1);//the size of the packet should always be just one byte
        if(status!=0){
          //free memory before exiting
          free(txBuffer->data); //free the data buffer in the structure
          free(txBuffer); //free the data structure
          continue;
        }
        //now write the data
        status = cc120xSpiWriteTxFifo(txBuffer->data, txBuffer->length);//
        if(status !=0){
          //free memory before exiting
          free(txBuffer->data); //free the data buffer in the structure
          free(txBuffer); //free the data structure
          continue;
        }
 
        // trigger transmission: send TX Strobe command
        status = cc120xSpiCmdStrobe(CC120X_STX);
        // wait for transmission to complete

        
        
        // free after transmission
        free(txBuffer->data);
        free(txBuffer);

      }
    }
  }
}


