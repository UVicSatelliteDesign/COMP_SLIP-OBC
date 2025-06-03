#ifndef TTC_INTERFACE_H
#define TTC_INTERFACE_H



#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"



typedef uint8_t FIFOsize[128];
extern volatile bool dirtyBit;
extern osMessageQueueId_t receivequeueHandle;
extern osSemaphoreId_t myBinarySem01Handle;









#endif
