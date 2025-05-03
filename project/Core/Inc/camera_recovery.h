/*
 * camera_recovery.h
 *
 *  Created on: Apr 16, 2025
 *      Author: noelvillarin357
 */

#ifndef INC_CAMERA_RECOVERY_H_
#define INC_CAMERA_RECOVERY_H_

// Number of tries for recovering from an error
#define MAX_ATTEMPTS 3

/**
 * @brief Recovery function for OV5640 register write operations.
 *
 * This function attempts to write data to the specified OV5640 register multiple times
 * until a successful write is achieved or the maximum number of attempts is reached.
 *
 * @param[in] camera Pointer to the Camera_t structure.
 * @param[in] func Function pointer for register write operation.
 * @param[in] reg Register address to write to.
 * @param[in] data Pointer to the data to write.
 * @param[in] length Length of the data to write.
 *
 * @return Status code.
 * @retval OV5640_OK Write operation was successful.
 * @retval OV5640_ERROR Write operation failed after retries.
 */
int32_t OV5640_Write_Reg_Recovery(void *camera, int32_t (*func)(void *camera, uint16_t reg, uint8_t *data, uint16_t length),
                                 uint16_t reg, uint8_t *data, uint16_t length);

/**
 * @brief Recovery function for OV5640 initialization.
 *
 * This function attempts to initialize the OV5640 camera with the given resolution and
 * pixel format multiple times until successful or the maximum number of attempts is reached.
 *
 * @param[in] camera Pointer to the Camera_t structure.
 * @param[in] func Function pointer for initialization.
 * @param[in] Resolution Resolution to set.
 * @param[in] PixelFormat Pixel format to set.
 *
 * @return Status code.
 * @retval OV5640_OK Initialization was successful.
 * @retval OV5640_ERROR Initialization failed after retries.
 */
int32_t OV5640_Init_Recovery(void *camera, int32_t (*func)(void *camera, uint32_t Resolution, uint32_t PixelFormat),
                            uint32_t Resolution, uint32_t PixelFormat);

/**
 * @brief Recovery function for OV5640 camera ID reading.
 *
 * This function attempts to read the camera ID multiple times until a successful read is achieved
 * or the maximum number of attempts is reached.
 *
 * @param[in] camera Pointer to the Camera_t structure.
 * @param[in] func Function pointer for ID reading.
 * @param[out] Id Pointer to store the camera ID.
 *
 * @return Status code.
 * @retval OV5640_OK ID read operation was successful.
 * @retval OV5640_ERROR ID read operation failed after retries.
 */
int32_t OV5640_ReadID_Recovery(void *camera, int32_t (*func)(void *camera, uint32_t *Id), uint32_t *Id);

/**
 * @brief Recovery function for OV5640 register bus I/O.
 *
 * This function attempts to configure the OV5640 camera bus I/O multiple times
 * until successful or the maximum number of attempts is reached.
 *
 * @param[in] camera Pointer to the Camera_t structure.
 * @param[in] func Function pointer for register bus I/O operation.
 * @param[in] camera_io Pointer to the OV5640_IO_t structure to configure.
 *
 * @return Status code.
 * @retval OV5640_OK I/O operation was successful.
 * @retval OV5640_ERROR I/O operation failed after retries.
 */
int32_t OV5640_RegisterBusIO_Recovery(void *camera, int32_t(*func)(void *camera, OV5640_IO_t *camera_io), OV5640_IO_t *camera_io);

#endif /* INC_CAMERA_RECOVERY_H_ */
