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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"
#include "ttc.h"
#include "obc.h"

  /* Private includes ----------------------------------------------------------*/
  /* USER CODE BEGIN Includes */

#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "stdio.h"
#include "semphr.h"
#include "timers.h"

  /* USER CODE END Includes */

  /* Exported types ------------------------------------------------------------*/
  /* USER CODE BEGIN ET */

  /* USER CODE END ET */

  /* Exported constants --------------------------------------------------------*/
  /* USER CODE BEGIN EC */
// Retransmission software timer
TimerHandle_t retransmission_timer;
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
  void receive(void *vpParameters);

  /* Definitions for retransmission software timer */
  void vRetransmissionTimerCallback( TimerHandle_t xTimer );

  /* USER CODE END EFP */

  /* Private defines -----------------------------------------------------------*/

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define TemperatureSensor_Pin GPIO_PIN_2
#define TemperatureSensor_GPIO_Port GPIOC
#define PressureSensor_Pin GPIO_PIN_3
#define PressureSensor_GPIO_Port GPIOC
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define Transciever_exti_Pin GPIO_PIN_13
#define Transciever_exti_GPIO_Port GPIOF
#define Transciever_exti_EXTI_IRQn EXTI15_10_IRQn
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB

#define OBC_Temperature_Pin GPIO_PIN_2
#define OBC_Temperature_GPIO_Port GPIOC
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
#define Transceiver_NSS_Pin GPIO_PIN_11
#define Transceiver_NSS_GPIO_Port GPIOE
#define Transceiver_SCK_Pin GPIO_PIN_12
#define Transceiver_SCK_GPIO_Port GPIOE
#define Transceiver_MISO_Pin GPIO_PIN_13
#define Transceiver_MISO_GPIO_Port GPIOE
#define Transceiver_MOSI_Pin GPIO_PIN_14
#define Transceiver_MOSI_GPIO_Port GPIOE
#define Memory_NSS_Pin GPIO_PIN_12
#define Memory_NSS_GPIO_Port GPIOB
#define Memory_SCK_Pin GPIO_PIN_13
#define Memory_SCK_GPIO_Port GPIOB
#define Memory_MISO_Pin GPIO_PIN_14
#define Memory_MISO_GPIO_Port GPIOB
#define Memory_MOSI_Pin GPIO_PIN_15
#define Memory_MOSI_GPIO_Port GPIOB

#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD
#define GPS_RX_Pin GPIO_PIN_0
#define GPS_RX_GPIO_Port GPIOD
#define GPS_TX_Pin GPIO_PIN_1
#define GPS_TX_GPIO_Port GPIOD
#define Transceiver_exti_Pin GPIO_PIN_5
#define Transceiver_exti_GPIO_Port GPIOD

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

#define NOMINAL 0x0010
#define LOW_POWER 0x0020

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
