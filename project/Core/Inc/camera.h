/*
 * camera.h
 *
 *  Created on: Feb 26, 2025
 *      Author: golde
 */

#include "stm32h7xx_hal.h"
#include "ov5640_reg.h"
#include "ov5640.h"

#ifndef INC_CAMERA_H_
#define INC_CAMERA_H_

#define OV5640_I2C_ADDRESS  (0x3C << 1) // 0x78

#define IMAGE_WIDTH 640 /**< Image width in pixels. */
#define IMAGE_HEIGHT 480 /**< Image height in pixels. */
#define JPEG_COMPRESSION_RATIO 0.8 /**< JPEG compression ratio. Modify as needed. */

// image buffer size for camera1
#define IMAGE_SIZE_1 (IMAGE_WIDTH * IMAGE_HEIGHT * 3 * JPEG_COMPRESSION_RATIO) /**< Size of the image buffer in bytes (for 640x480, JPEG, Colour). */

// image buffer size for camera2
#define IMAGE_SIZE_2 (IMAGE_WIDTH * IMAGE_HEIGHT * 3 * JPEG_COMPRESSION_RATIO) /**< Size of the image buffer in bytes (for 640x480, JPEG, Colour). */

// MAX IMAGE BUFFER SIZE - choose larger image between camera 1 and 2 (assume camera1 has the larger image size for now)
#define MAX_IMAGE_BUFFER_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 3 * JPEG_COMPRESSION_RATIO)

/**
 * @struct Camera_t
 * @brief Camera configuration structure.
 *
 * This structure contains the settings, image properties, and other information for
 * managing a camera, including the camera's context and GPIO configuration.
 */
typedef struct Camera_t {

    int32_t status; /**< Current status of the camera. */

    /** Image settings */
    uint32_t Resolution; /**< Camera resolution. */
    uint32_t PixelFormat; /**< Pixel format. */
    uint32_t PclkPolarity; /**< Polarity of pixel clock. */
    uint32_t HsyncPolarity; /**< Polarity of horizontal sync. */
    uint32_t VsyncPolarity; /**< Polarity of vertical sync. */
    OV5640_Capabilities_t Capabilities; /**< Camera capabilities. */
    uint32_t LightMode; /**< Light mode (e.g., automatic). */
    uint32_t Effect; /**< Camera effect settings. */
    int32_t Brightness; /**< Brightness setting. */
    int32_t Saturation; /**< Saturation setting. */
    int32_t Contrast; /**< Contrast setting. */
    int32_t Hue; /**< Hue setting. */
    uint32_t Config; /**Camera mirror/vflip setting. */
    uint32_t Zoom; /**< Zoom level. */
    uint32_t NightMode; /**< Night mode setting. */
    OV5640_SyncCodes_t pSyncCodes; /**< Sync codes for the camera. */

    /** Other settings */
    uint32_t ClockValue; /**< Pixel clock value. */
    uint8_t* imageBuffer; /**< Pointer to the image buffer where captured images are stored. */
    uint32_t imageSize; /**< Estimated image size */
    uint32_t actualImageSize; /** < Actual image size after capture */

    /** Camera identifier and context */
    uint32_t cameraId; /**< Unique camera ID. */
    OV5640_Object_t cameraHandle; /**< Camera handle for managing the camera. */

    /** GPIO configuration */
    // cameras should be connected to the same gpio pins
    uint16_t gpioPin; /**< GPIO pin connected to the camera. */
    GPIO_TypeDef *gpioPort; /**< GPIO port for the camera. */

    /** Camera context for managing the camera's registers */
    ov5640_ctx_t ctx; /**< OV5640 context structure. */ // initialize ctx.handle with the relevant i2c bus

} Camera_t;

/* Function declarations */

/**
 * @brief Configures camera 1 with default settings
 * @param[in,out] camera Pointer to the Camera_t structure to configure
 * @param[in] buffer Pointer to the image buffer to store captured data
 * @details Configures:
 * - I2C communication handle
 * - Resolution (640x480)
 * - Pixel format (JPEG)
 * - Synchronization polarities
 * - Image size
 * - Pixel clock (48MHz)
 * @return Status code
 * @retval OV5640_OK Configuration successful
 * @retval OV5640_ERROR Configuration failed
 */
int32_t configure_camera1(Camera_t *camera, uint8_t *buffer);

/**
 * @brief Configures camera 2 with default settings
 * @param[in,out] camera Pointer to the Camera_t structure to configure
 * @param[in] buffer Pointer to the image buffer to store captured data
 * @details Configures:
 * - I2C communication handle
 * - Resolution (640x480)
 * - Pixel format (JPEG)
 * - Synchronization polarities
 * - Image size
 * - Pixel clock (48MHz)
 * @return Status code
 * @retval OV5640_OK Configuration successful
 * @retval OV5640_ERROR Configuration failed
 */
int32_t configure_camera2(Camera_t *camera, uint8_t *buffer);

/**
 * @brief Initializes the camera with the provided configuration.
 *
 * This function sets up the camera hardware and configures it based on the settings
 * provided in the Camera_t structure.
 *
 * @param camera Pointer to the Camera_t structure containing camera configuration.
 * @return int32_t Returns 0 on success, or an error code if initialization fails.
 */
int32_t camera_init(Camera_t *camera);

/**
 * @brief Saves the captured image to flash memory.
 *
 * This function writes the image stored in the camera's image buffer to a specific
 * sector in the flash memory.
 *
 * @param camera Pointer to the Camera_t structure containing the image buffer.
 * @param sector The flash memory sector where the image will be saved.
 */
void save_image_to_flash(Camera_t *camera, uint32_t sector);

/**
 * @brief Captures a snapshot from two cameras based on the GPIO state.
 *
 * This function captures images from two cameras simultaneously or sequentially,
 * depending on the state of the GPIO pin provided.
 *
 * @param camera1 Pointer to the Camera_t structure for the first camera.
 * @param camera2 Pointer to the Camera_t structure for the second camera.
 * @param gpio_state The state of the GPIO pin that triggers the capture.
 */
HAL_StatusTypeDef capture_snapshot(Camera_t *camera);

/**
 * @brief Gets the actual size of the captured image
 * @param[in] camera Pointer to the Camera_t structure
 * @return Actual size of the image in bytes
 * @retval 0 if invalid parameters or JPEG markers not found
 * @details Checks for:
 * - JPEG start marker (0xFFD8)
 * - JPEG end marker (0xFFD9)
 */
uint32_t getImageSize(Camera_t *camera);

/** 
* @brief Deallocates image buffer memory
* @param[in,out] camera Pointer to the Camera_t structure
*/
void freeImageBuffer(Camera_t *camera);

#endif /* INC_CAMERA_H_ */
