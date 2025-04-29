/**
 * @file camera_snapshot.c
 * @brief Camera snapshot functionality for capturing images from OV5640 cameras.
 *
 * This module provides functions to initialize, configure, and capture images from OV5640 cameras.
 * It supports dual camera operation with recovery mechanisms for failed operations.
 *
 * @details The implementation includes:
 * - Camera initialization and configuration
 * - Image capture and buffer management
 * - Flash memory storage
 * - Error recovery mechanisms
 * - Hardware abstraction for STM32H7xx microcontrollers
 *
 * @date Created on: Feb 26, 2025
 * @author noelvillarin357
 * @version 1.0
 * @note Requires HAL library for STM32H7xx and OV5640 driver
 */

#include "ttc.h"
#include "ttc_interface.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdlib.h>

#include "ov5640.h"
#include "ov5640_reg.h"

#include "camera.h"
#include "camera_recovery.h"
#include "camera_bsp.h"

/** @def SWITCH_CAMERA(camera, state)
 *  @brief Macro to switch camera state via GPIO
 *  @param camera Camera structure containing GPIO information
 *  @param state GPIO state to set (GPIO_PIN_SET or GPIO_PIN_RESET)
 */
#define SWITCH_CAMERA(camera, state)  HAL_GPIO_WritePin(camera->gpioPort, camera->gpioPin, state)

/**
 * @var capture_complete_flag
 * @brief Flag to indicate capture completion status
 * @note Volatile as it's modified in interrupt context
 */
volatile uint8_t capture_complete_flag = 0;

extern DCMI_HandleTypeDef hdcmi; /**< External DCMI handle declaration */

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
int32_t configure_camera1(Camera_t *camera, uint8_t *buffer) {

	// define i2c functions the OV5640 driver will use
	OV5640_IO_t camera1_io = {
	    .Init     = BSP_I2C2_Init,
	    .DeInit   = BSP_I2C2_DeInit,
	    .Address  = OV5640_I2C_ADDRESS,
	    .WriteReg = BSP_I2C2_WriteReg,
	    .ReadReg  = BSP_I2C2_ReadReg,
	    .GetTick  = BSP_GetTick
	};

	int32_t status = OV5640_RegisterBusIO(&camera->cameraHandle, &camera1_io);

	// attempt recovery
	if (status != OV5640_OK) {
		status = OV5640_RegisterBusIO_Recovery(camera, (int32_t (*)(void *, OV5640_IO_t *))OV5640_RegisterBusIO, &camera1_io);
		if (status != OV5640_OK) {
			return status;
		}
	}

    // Set resolution and format
    camera->Resolution = OV5640_R640x480;
    camera->PixelFormat = OV5640_JPEG;

    // Sync polarities
    camera->PclkPolarity = OV5640_POLARITY_PCLK_LOW;
    camera->HsyncPolarity = OV5640_POLARITY_HREF_LOW;
    camera->VsyncPolarity = OV5640_POLARITY_VSYNC_LOW;

    // Set image size
    camera->imageSize = IMAGE_SIZE_1;

    // Give reference to image buffer
    camera->imageBuffer = buffer;

    // Set pixel clock value
    camera->ClockValue = OV5640_PCLK_48M;   // 48MHz

	 // OPTIONAL IMAGE SETTINGS
	 // TODO: Configure these later
	 // camera->LightMode
	 // camera->Effect
	 // camera->Brightness
	 // camera->Saturation
	 // camera->Contrast
	 // camera->Hue
	 // camera->Config
	 // camera->Zoom
	 // camera->NightMode

    return OV5640_OK;
}

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
int32_t configure_camera2(Camera_t *camera, uint8_t *buffer) {

	// define i2c functions the OV5640 driver will use
	OV5640_IO_t camera2_io = {
	    .Init     = BSP_I2C4_Init,
	    .DeInit   = BSP_I2C4_DeInit,
	    .Address  = OV5640_I2C_ADDRESS,
	    .WriteReg = BSP_I2C4_WriteReg,
	    .ReadReg  = BSP_I2C4_ReadReg,
	    .GetTick  = BSP_GetTick
	};

	int32_t status = OV5640_RegisterBusIO(&camera->cameraHandle, &camera2_io);

	// attempt recovery
	if (status != OV5640_OK) {
		status = OV5640_RegisterBusIO_Recovery(camera, (int32_t (*)(void *, OV5640_IO_t *))OV5640_RegisterBusIO, &camera2_io);
		if (status != OV5640_OK) {
			return status;
		}
	}

    // Set resolution and format
    camera->Resolution = OV5640_R640x480;
    camera->PixelFormat = OV5640_JPEG;

    // Sync polarities
    camera->PclkPolarity = OV5640_POLARITY_PCLK_LOW;
    camera->HsyncPolarity = OV5640_POLARITY_HREF_LOW;
    camera->VsyncPolarity = OV5640_POLARITY_VSYNC_LOW;

    // Set image size
    camera->imageSize = IMAGE_SIZE_2;

    // Give reference to image buffer
    camera->imageBuffer = buffer;

    // Set pixel clock value
    camera->ClockValue = OV5640_PCLK_48M;   // 48MHz

	 // OPTIONAL IMAGE SETTINGS
	 // TODO: Configure these later
	 // camera->LightMode
	 // camera->Effect
	 // camera->Brightness
	 // camera->Saturation
	 // camera->Contrast
	 // camera->Hue
	 // camera->Config
	 // camera->Zoom
	 // camera->NightMode

    return OV5640_OK;
}

/**
 * @brief Initializes the camera with default resolution and pixel format
 * @param[in,out] camera Pointer to the Camera_t structure
 * @return Status code of the initialization process
 * @retval OV5640_OK Initialization successful
 * @retval OV5640_ERROR Initialization failed
 * @details Performs:
 * - Camera reset
 * - Block enable
 * - ID verification
 * - Default configuration (640x480, JPEG)
 */
int32_t camera_init(Camera_t *camera) {
    int32_t status;

    // reset the camera
    uint8_t systemControlRegValue = 0x80;
    status = ov5640_write_reg(&camera->ctx, OV5640_SYSTEM_CTROL0, &systemControlRegValue, 1);

    // attempt recovery if error occurs
    if (status != OV5640_OK) {
        status = OV5640_Write_Reg_Recovery(camera, (int32_t (*)(void *, uint16_t, uint8_t *, uint16_t))ov5640_write_reg,
                                          OV5640_SYSTEM_CTROL0, &systemControlRegValue, 1);
        if (status != OV5640_OK) {
            return status;
        }
    }

    HAL_Delay(200); // 200ms delay to allow reset

    // enable blocks
    uint8_t resetControlRegValue = 0x00;
    status = ov5640_write_reg(&camera->ctx, OV5640_SYSREM_RESET00, &resetControlRegValue, 1);

    // attempt recovery if error occurs
    if (status != OV5640_OK) {
        status = OV5640_Write_Reg_Recovery(camera, (int32_t (*)(void *, uint16_t, uint8_t *, uint16_t))ov5640_write_reg,
                                          OV5640_SYSTEM_CTROL0, &systemControlRegValue, 1);
        if (status != OV5640_OK) {
            return status;
        }
    }

    // set compression level
    uint8_t compressionRegValue = 0x20; // ~(70-80%)
    status = ov5640_write_reg(&camera->ctx, OV5640_JPEG_CTRL07, &compressionRegValue, 1);

    // attempt recovery if error occurs
    if (status != OV5640_OK) {
        status = OV5640_Write_Reg_Recovery(camera, (int32_t (*)(void *, uint16_t, uint8_t *, uint16_t))ov5640_write_reg,
                                          OV5640_JPEG_CTRL07, &compressionRegValue, 1);
        if (status != OV5640_OK) {
            return status;
        }
    }

    /* Initialize camera with 640x480 resolution and JPEG format */
    status = OV5640_Init(&camera->cameraHandle, camera->Resolution, camera->PixelFormat);

    if (status != OV5640_OK) {
        status = OV5640_Init_Recovery(camera, (int32_t (*)(void *, uint32_t, uint32_t))OV5640_Init,
                                    camera->Resolution, camera->PixelFormat);
        if (status != OV5640_OK) {
            return status;
        }
    }

    uint32_t cameraId;
    /* Read camera ID */
    status = OV5640_ReadID(&camera->cameraHandle, &cameraId);

    if (status != OV5640_OK) {
        status = OV5640_ReadID_Recovery(camera, (int32_t (*)(void *, uint32_t *))OV5640_ReadID, &cameraId);
        if (status != OV5640_OK) {
            return status;
        }
    }

    camera->cameraId = cameraId;

	 // TODO: configure image settings later
	 // Call these functions later
	 // OV5640_SetLightMode(&camera->cameraHandle, camera->LightMode);
	 // OV5640_SetColorEffect(&camera->cameraHandle, camera->Effect);
	 // OV5640_SetBrightness(&camera->cameraHandle, camera->Brightness);
	 // OV5640_SetSaturation(&camera->cameraHandle, camera->Saturation);
	 // OV5640_SetContrast(&camera->cameraHandle, camera->Contrast);
	 // OV5640_SetHueDegree(&camera->cameraHandle, camera->Hue);
	 // OV5640_MirrorFlipConfig(&camera->cameraHandle, camera->Config);
	 // OV5640_ZoomConfig(&camera->cameraHandle, camera->Zoom);
	 // OV5640_NightModeConfig(&camera->cameraHandle, camera->NightMode);


    OV5640_Start(&camera->cameraHandle);
    camera->status = OV5640_OK;
    return OV5640_OK;
}

/**
 * @brief Saves the captured image to flash memory
 * @param[in] camera Pointer to the Camera_t structure
 * @param[in] sector The flash memory sector to save the image to
 * @details Performs:
 * - Flash memory unlock
 * - Sector erase
 * - Image data programming
 * - Flash memory lock
 * @note Frees the image buffer after saving
 */
void save_image_to_flash(Camera_t *camera, uint32_t sector) {
    if (camera->imageBuffer == NULL){
        // Handle error if image buffer is empty
        return;
    }

    uint32_t sectorError;

    // unlock flash memory
    HAL_FLASH_Unlock();

    HAL_FLASHEx_Erase(&(FLASH_EraseInitTypeDef){
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Sector = sector,  // Sector to erase (e.g., FLASH_SECTOR_2 or FLASH_SECTOR_3)
        .NbSectors = 1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    }, &sectorError);

    if (sectorError != 0xFFFFFFFF) {
        // Handle error
        HAL_FLASH_Lock();
        return;
    }

    uint32_t flashAddress = FLASH_BASE + (sector * FLASH_SECTOR_SIZE);
    for (uint32_t i = 0; i < camera->imageSize / 4; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flashAddress + (i * 4),
                          ((uint32_t *)camera->imageBuffer)[i]);
    }

    // lock flash memory
    HAL_FLASH_Lock();

    // empty image buffer to save RAM
    free(camera->imageBuffer);
    camera->imageBuffer = NULL;
}

/**
 * @brief Initiates the snapshot capture process
 * @param[in,out] camera Pointer to the camera structure to capture the image
 * @return HAL status
 * @retval HAL_OK Capture successful
 * @retval HAL_TIMEOUT Capture timed out
 * @retval HAL_ERROR Memory allocation or capture failed
 * @details Performs:
 * - Camera start
 * - DCMI readiness check
 * - Image buffer allocation
 * - DMA capture initiation
 * - Capture completion wait
 * - Camera stop
 */
HAL_StatusTypeDef capture_snapshot(Camera_t *camera){
    OV5640_Start(&camera->cameraHandle);

    // Wait for DCMI to be ready with a timeout
    uint32_t timeout = HAL_GetTick() + 1000; // 1 second timeout
    while (HAL_DCMI_GetState(&hdcmi) != HAL_DCMI_STATE_READY) {
        if (HAL_GetTick() > timeout) {
            return HAL_TIMEOUT;
        }
    }

    capture_complete_flag = 0;

    // Allocate memory for image buffer if not already allocated
    if (camera->imageBuffer == NULL) {
        for (int numAttempts = 0; numAttempts < MAX_ATTEMPTS; numAttempts++) {
            camera->imageBuffer = malloc(MAX_IMAGE_BUFFER_SIZE);
            if (camera->imageBuffer != NULL) {
                break;
            }
            HAL_Delay(50);
        }

        if (camera->imageBuffer == NULL) {
            return HAL_ERROR;
        }
    } else {
        free(camera->imageBuffer);
        camera->imageBuffer = NULL;

        for (int numAttempts = 0; numAttempts < MAX_ATTEMPTS; numAttempts++) {
            camera->imageBuffer = malloc(MAX_IMAGE_BUFFER_SIZE);
            if (camera->imageBuffer != NULL) {
                break;
            }
            HAL_Delay(50);
        }

        if (camera->imageBuffer == NULL) {
            return HAL_ERROR;
        }
    }

    HAL_StatusTypeDef status;
    for (int numAttempts = 0; numAttempts < MAX_ATTEMPTS; numAttempts++) {
        status = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)camera->imageBuffer, camera->imageSize / 4);
        if (status == HAL_OK) {
            break;
        }
        HAL_Delay(50);
    }

    if (status != HAL_OK) {
        return status;
    }

    while (capture_complete_flag == 0) {
        // wait for capture to complete
    }

    OV5640_Stop(&camera->cameraHandle);
    camera->actualImageSize = getImageSize(camera);
    return status;
}

/**
 * @brief DCMI frame event callback
 * @param[in] hdcmi Pointer to the DCMI_HandleTypeDef structure
 * @note Sets capture_complete_flag when frame capture is done
 */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi) {
    capture_complete_flag = 1; // Set flag when capture is done
}

/**
 * @brief Deallocates image buffer memory
 * @param[in,out] camera Pointer to the Camera_t structure
 */
void freeImageBuffer(Camera_t *camera) {
    free(camera->imageBuffer);
    camera->imageBuffer = NULL;
}

/**
 * @brief Gets the actual size of the captured image
 * @param[in] camera Pointer to the Camera_t structure
 * @return Actual size of the image in bytes
 * @retval 0 if invalid parameters or JPEG markers not found
 * @details Checks for:
 * - JPEG start marker (0xFFD8)
 * - JPEG end marker (0xFFD9)
 */
uint32_t getImageSize(Camera_t *camera) {
    uint8_t *temp = camera->imageBuffer;
    uint32_t maxSize = camera->imageSize;
    uint32_t actualSize = 0;

    // Check if camera is not initialized, image buffer is null, and minimum image size
    if (!camera || !camera->imageBuffer || camera->imageSize < 4) {
        return 0;
    }

    // check for jpeg start marker (0xFFD8)
    if (temp[0] != 0xFF || temp[1] != 0xD8) {
        return 0;
    }

    // search for jpeg end marker (0xFFD9)
    for (actualSize = 0; actualSize < maxSize - 1; actualSize++) {
        if (temp[actualSize] == 0xFF && temp[actualSize+1] == 0xD9) {
            actualSize += 2; // Include both bytes of the end marker
            return actualSize;
        }
    }

    return actualSize;
}
