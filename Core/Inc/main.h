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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_SPI_CLK_Pin GPIO_PIN_2
#define LCD_SPI_CLK_GPIO_Port GPIOE
#define LT_RIGHT_DI_Pin GPIO_PIN_3
#define LT_RIGHT_DI_GPIO_Port GPIOE
#define LT_RIGHT_CI_Pin GPIO_PIN_4
#define LT_RIGHT_CI_GPIO_Port GPIOE
#define LCD_SPI_MISO_Pin GPIO_PIN_5
#define LCD_SPI_MISO_GPIO_Port GPIOE
#define LCD_SPI_MOSI_Pin GPIO_PIN_6
#define LCD_SPI_MOSI_GPIO_Port GPIOE
#define LCD_GPIO0_Pin GPIO_PIN_13
#define LCD_GPIO0_GPIO_Port GPIOC
#define LCD_GPIO1_Pin GPIO_PIN_14
#define LCD_GPIO1_GPIO_Port GPIOC
#define LCD_GPIO2_Pin GPIO_PIN_2
#define LCD_GPIO2_GPIO_Port GPIOC
#define EVE_PD_NOT_Pin GPIO_PIN_0
#define EVE_PD_NOT_GPIO_Port GPIOB
#define EVE_CS_NOT_Pin GPIO_PIN_1
#define EVE_CS_NOT_GPIO_Port GPIOB
#define PWM1_Pin GPIO_PIN_9
#define PWM1_GPIO_Port GPIOE
#define MAX_CS_Pin GPIO_PIN_12
#define MAX_CS_GPIO_Port GPIOB
#define LT_LEFT_DI_Pin GPIO_PIN_12
#define LT_LEFT_DI_GPIO_Port GPIOD
#define LT_LEFT_CI_Pin GPIO_PIN_14
#define LT_LEFT_CI_GPIO_Port GPIOD
#define LT_BAR_DI_Pin GPIO_PIN_9
#define LT_BAR_DI_GPIO_Port GPIOA
#define LT_BAR_CI_Pin GPIO_PIN_11
#define LT_BAR_CI_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
