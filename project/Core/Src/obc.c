#include "obc.h"
#include "camera.h"
#include "obc_interface.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdlib.h>

void obc_notifications(void *vpParameters) {
    uint32_t received_notification;
    Camera_t camera;

    configure_camera1(&camera, NULL);
    camera_init(&camera);

    for (;;) {
        received_notification = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (received_notification & REQUEST_CAMERA) {
            if (capture_snapshot(&camera) == HAL_OK) {
                // Save to SD card
                store_image(camera.imageBuffer);

                // Save to flash
                save_image_to_flash(&camera, FLASH_SECTOR_2);

                // Free image buffer
                free(camera.imageBuffer);
            } 
            else log_error("Camera capture failed");
        }
    }
}
