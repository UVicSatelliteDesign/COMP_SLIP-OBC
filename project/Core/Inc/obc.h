#ifndef OBC_H
#define OBC_H

#include "FreeRTOS.h"
#include "task.h"

#define IDLE 0
#define NOMINAL 1
#define LOW_POWER 2

#define NOMINAL_INTERVAL 20000 // 1000 = 1 second
#define LOW_POWER_INTERVAL 60000

void obc_notifications(void *vpParameters);

#endif // OBC_H
