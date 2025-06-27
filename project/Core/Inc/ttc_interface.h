#ifndef TTC_INTERFACE_H
#define TTC_INTERFACE_H



#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"



typedef uint8_t FIFOsize[128];  // new typedef used for defining FIFO size as mentioned in the CC1201 datasheet
extern volatile bool dirtyBit;	// the bool dirty bit originally set to false
extern osMessageQueueId_t receivequeueHandle; // the message queue used for receiving info from the trasnciever
extern osSemaphoreId_t myBinarySem01Handle;   // used to protect the dirty bit. it is a sempahore

/////// get_gps() will need a way to pass data, either a write to flash function or a return type
void get_gps(); // This uses UART to receive a $GPRMC gps data sentence from the gps and write it to a buffer








#endif
