#include "obc.h"

#include <math.h>

#include "cmsis_os.h"

void obc_notifications(void *vpParameters) {
    uint32_t received_notification;
    
    while (1) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & REQUEST_CAMERA) {
            // TODO
        }
    }
}

