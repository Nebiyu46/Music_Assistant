/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "song.h"

void Error_Handler(void);

#define TFT_RES_Pin GPIO_PIN_15
#define TFT_RES_GPIO_Port GPIOA
#define TFT_DC_Pin GPIO_PIN_3
#define TFT_DC_GPIO_Port GPIOB
#define TFT_CS_Pin GPIO_PIN_6
#define TFT_CS_GPIO_Port GPIOB

#define BTN_OK_GPIO_Port GPIOA
#define BTN_OK_Pin GPIO_PIN_1

#define BTN_NAV_GPIO_Port GPIOA
#define BTN_NAV_Pin GPIO_PIN_2

#define START_GAME_GPIO_Port BTN_OK_GPIO_Port
#define START_GAME_Pin BTN_OK_Pin

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
