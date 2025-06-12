#include "obc.h"

#include <math.h>

#include "cmsis_os.h"
#include "camera.h"
#include <string.h>


osMessageQueueId_t imageQueueHandle;



void obc_notifications(void *vpParameters) {
    uint32_t received_notification;

    // Create the image queue and storage task
    imageQueueHandle = osMessageQueueNew(IMAGE_QUEUE_LENGTH, sizeof(uint8_t *), NULL);
    xTaskCreate(image_storage, "ImageStore", 512, NULL, tskIDLE_PRIORITY + 2, NULL);


    while (1) {
        received_notification = ulTaskNotifyTake(pdFALSE, 0);

        if (received_notification & REQUEST_CAMERA) {
            // Trigger camera capture
        }
    }
}

<<<<<<< HEAD
/**
 * @todo create an RTOS object that stores, gives the signal that its done, when the RTOS object
 * is called, keep requesting.  
 * @todo read up on task notifications and queues 
 */

/**
 * @brief  Function to store images in flash memory
 * @param  arg: argument passed to the thread  
 * @return None
 */
 void image_storage(void *arg) {
    uint8_t *image_buffer;
    for(;;) {
        if(osMessageQueueGet(imageQueueHandle, &image_buffer, osWaitForever) == osOK) {
            // Store the image in flash memory
            store_image(image_buffer);
        }
    }
}

/**
 * @brief  Function to send images to the OBC
 * @param  image_buffer: pointer to the image buffer
 * @return None
 */
void obc_send_image(uint8_t *image_buffer){
    if (imageQueueHandle != NULL) {
        osMessageQueuePut(imageQueueHandle, &image_buffer, 0, 0);
    }
}
=======
>>>>>>> 7e173d740136134bb848edb303a15efe936b312f
