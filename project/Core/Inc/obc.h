#ifndef OBC_H
#define OBC_H

#include "FreeRTOS.h"
#include "task.h"

#define IDLE 0
#define NOMINAL 1
#define LOW_POWER 2

void obc_notifications(void *vpParameters);

#endif // OBC_H
