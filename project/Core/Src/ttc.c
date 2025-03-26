#include "ttc.h"
#include "cmsis_os.h"

void ttc_notifications(void *vpParameters) {
    uint32_t received_notification = 0;
    
    for (;;) {
        // Non-blocking check for notifications with a 0ms timeout
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & CAMERA_READY) {
            // TODO
        }
        received_notification = 0;
    }
}
