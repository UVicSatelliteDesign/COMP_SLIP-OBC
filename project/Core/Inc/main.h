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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TemperatureSensor_Pin GPIO_PIN_2
#define TemperatureSensor_GPIO_Port GPIOC
#define PressureSensor_Pin GPIO_PIN_3
#define PressureSensor_GPIO_Port GPIOC
#define CAM1_PD_Pin GPIO_PIN_0
#define CAM1_PD_GPIO_Port GPIOA
#define CAM1_RT_Pin GPIO_PIN_1
#define CAM1_RT_GPIO_Port GPIOA
#define CAM_SEL_Pin GPIO_PIN_2
#define CAM_SEL_GPIO_Port GPIOA
#define CAM2_PD_Pin GPIO_PIN_3
#define CAM2_PD_GPIO_Port GPIOA
#define CAM2_RT_Pin GPIO_PIN_5
#define CAM2_RT_GPIO_Port GPIOA
#define Transciever_exti_Pin GPIO_PIN_13
#define Transciever_exti_GPIO_Port GPIOF
#define Transciever_exti_EXTI_IRQn EXTI15_10_IRQn
#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD
#define USB_OTG_FS_PWR_EN_Pin GPIO_PIN_10
#define USB_OTG_FS_PWR_EN_GPIO_Port GPIOD
#define GPS_UART_Pin GPIO_PIN_1
#define GPS_UART_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
