#ifndef SRC_OBC_H_
#define SRC_OBC_H_

#include "stm32h7xx_hal.h"
#include "obc_interface.h"
// #include "cmsis_os.h"

// RTOS Queue for receiving battery data from BMS
extern osMessageQueueId_t bmsQueue;

// Function prototypes
void OBC_Task(void *argument);

#endif /* SRC_OBC_H_ */
