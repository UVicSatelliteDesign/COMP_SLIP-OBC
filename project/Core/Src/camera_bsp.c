/*
 * camera_bus.c
 *
 *  Created on: Apr 16, 2025
 *      Author: noelvillarin357
 */

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdlib.h>

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;

// I2C2 functions

/**
 * @brief Initializes the I2C2 peripheral.
 *
 * This function checks the current state of the I2C2 peripheral and returns the
 * initialization status. If I2C2 is not initialized, it returns an error code.
 *
 * @return Status code.
 * @retval 0 I2C2 is initialized.
 * @retval -1 I2C2 is not initialized.
 */
int32_t BSP_I2C2_Init(void) {
	if (HAL_I2C_GetState(&hi2c2) == HAL_I2C_STATE_RESET){
		return -1; // not initialized
	}

	return 0; // initialized
}

/**
 * @brief Deinitializes the I2C2 peripheral.
 *
 * This function attempts to deinitialize the I2C2 peripheral and returns the result.
 *
 * @return Status code.
 * @retval 0 Deinitialization successful.
 * @retval -1 Deinitialization failed.
 */
int32_t BSP_I2C2_DeInit(void) {
	HAL_StatusTypeDef result = HAL_I2C_DeInit(&hi2c2);

	if (result == HAL_OK){
		return 0;
	} else {
		return -1;
	}
}

/**
 * @brief Reads a register from I2C2.
 *
 * This function reads a specified register from an I2C device connected to I2C2.
 *
 * @param[in] address I2C device address.
 * @param[in] reg Register address to read from.
 * @param[out] pData Pointer to the buffer to store the read data.
 * @param[in] length Number of bytes to read.
 *
 * @return Status code.
 * @retval 0 Read operation successful.
 * @retval -1 Read operation failed.
 */
int32_t BSP_I2C2_ReadReg(uint16_t address, uint16_t reg, uint8_t *pData, uint16_t length){
	return (HAL_I2C_Mem_Read(&hi2c2, address, reg, I2C_MEMADD_SIZE_16BIT, pData, length, 1000) == HAL_OK) ? 0 : -1;
}

/**
 * @brief Writes to a register on I2C2.
 *
 * This function writes data to a specified register of an I2C device connected to I2C2.
 *
 * @param[in] address I2C device address.
 * @param[in] reg Register address to write to.
 * @param[in] pData Pointer to the data to write.
 * @param[in] length Number of bytes to write.
 *
 * @return Status code.
 * @retval 0 Write operation successful.
 * @retval -1 Write operation failed.
 */
int32_t BSP_I2C2_WriteReg(uint16_t address, uint16_t reg, uint8_t *pData, uint16_t length){
	return (HAL_I2C_Mem_Write(&hi2c2, address, reg, I2C_MEMADD_SIZE_16BIT, pData, length, 1000) == HAL_OK) ? 0 : -1;
}

// I2C4 functions

/**
 * @brief Initializes the I2C4 peripheral.
 *
 * This function checks the current state of the I2C4 peripheral and returns the
 * initialization status. If I2C4 is not initialized, it returns an error code.
 *
 * @return Status code.
 * @retval 0 I2C4 is initialized.
 * @retval -1 I2C4 is not initialized.
 */
int32_t BSP_I2C4_Init(void) {
	if (HAL_I2C_GetState(&hi2c4) == HAL_I2C_STATE_RESET){
		return -1; // not initialized
	}

	return 0; // initialized
}

/**
 * @brief Deinitializes the I2C4 peripheral.
 *
 * This function attempts to deinitialize the I2C4 peripheral and returns the result.
 *
 * @return Status code.
 * @retval 0 Deinitialization successful.
 * @retval -1 Deinitialization failed.
 */
int32_t BSP_I2C4_DeInit(void) {
	HAL_StatusTypeDef result = HAL_I2C_DeInit(&hi2c4);

	if (result == HAL_OK){
		return 0;
	} else {
		return -1;
	}
}

/**
 * @brief Reads a register from I2C4.
 *
 * This function reads a specified register from an I2C device connected to I2C4.
 *
 * @param[in] address I2C device address.
 * @param[in] reg Register address to read from.
 * @param[out] pData Pointer to the buffer to store the read data.
 * @param[in] length Number of bytes to read.
 *
 * @return Status code.
 * @retval 0 Read operation successful.
 * @retval -1 Read operation failed.
 */
int32_t BSP_I2C4_ReadReg(uint16_t address, uint16_t reg, uint8_t *pData, uint16_t length){
	return (HAL_I2C_Mem_Read(&hi2c4, address, reg, I2C_MEMADD_SIZE_16BIT, pData, length, 1000) == HAL_OK) ? 0 : -1;
}

/**
 * @brief Writes to a register on I2C4.
 *
 * This function writes data to a specified register of an I2C device connected to I2C4.
 *
 * @param[in] address I2C device address.
 * @param[in] reg Register address to write to.
 * @param[in] pData Pointer to the data to write.
 * @param[in] length Number of bytes to write.
 *
 * @return Status code.
 * @retval 0 Write operation successful.
 * @retval -1 Write operation failed.
 */
int32_t BSP_I2C4_WriteReg(uint16_t address, uint16_t reg, uint8_t *pData, uint16_t length){
	return (HAL_I2C_Mem_Write(&hi2c4, address, reg, I2C_MEMADD_SIZE_16BIT, pData, length, 1000) == HAL_OK) ? 0 : -1;
}

// other

/**
 * @brief Gets the current tick value.
 *
 * This function returns the current tick value from the HAL library, which represents
 * the number of milliseconds since the system started.
 *
 * @return Current tick value in milliseconds.
 */
int32_t BSP_GetTick(void) {
	return (int32_t)HAL_GetTick();
}
