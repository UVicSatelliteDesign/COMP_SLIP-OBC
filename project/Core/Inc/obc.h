#ifndef OBC_H
#define OBC_H

#include "FreeRTOS.h"
#include "task.h"

#define REQUEST_CAMERA 0x01
#define CAMERA_READY 0x02

void obc_notifications(void *vpParameters);

#endif // OBC_H
