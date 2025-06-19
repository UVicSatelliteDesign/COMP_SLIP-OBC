/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "stdio.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

void obc_notifications(void *vpParameters);
void ttc_notifications(void *vpParameters);
void data_task(void *vpParameters);
void low_power_task(void *vpParameters);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD
#define USB_OTG_FS_PWR_EN_Pin GPIO_PIN_10
#define USB_OTG_FS_PWR_EN_GPIO_Port GPIOD
#define USB_OTG_FS_OVCR_Pin GPIO_PIN_7
#define USB_OTG_FS_OVCR_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/*
 * Set mode from TTC: REQUEST & <MODE>
 * Notify TTC of mode change: INFO & <MODE>
 * Notify TTC of peripheral ready: INFO & <PERIPHERAL>
 */

// Message type

#define REQUEST 0x0001
#define INFO 0x0002
#define WARNING 0x0004
#define ERROR 0x0008

// Mode

#define IDLE 0x0010
#define NOMINAL 0x0020
#define LOW_POWER 0x0040

// Peripheral type

#define CAMERA 0x0100
#define SENSORS 0x0200
#define GPS 0x0400
#define MEMORY 0x0800

// Peripheral subtype

#define SUB_1 0x1000 // Camera 1,	Temperature,	Memory Read
#define SUB_2 0x2000 // Camera 2,	Pressure,		Memory Write
#define SUB_3 0x4000 // 			Acceleration
#define SUB_4 0x8000 // 			Gyroscope

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
