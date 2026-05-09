/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE BEGIN Private defines */

// Max number of notes to store in RAM
#define MAX_SONG_NOTES 300 
void Parse_Song_From_Buffer(void);

/* USER CODE END Private defines */
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

#define SONG_KEYS_PER_SECTION 8

typedef struct {
    uint16_t start_ms;      // song-relative onset time
    uint16_t duration_ms;   // sustain length
    uint8_t  midi_note;     // exact MIDI value to validate against
    uint8_t  key_idx;       // 0..SONG_KEYS_PER_SECTION-1 display column
} SongNote_t;

typedef struct {
    uint16_t first_note_idx;                       // index into Song_t.notes[]
    uint8_t  base_midi;                            // for reference / future use
    const char* labels[SONG_KEYS_PER_SECTION];     // per-key display labels
} SongSection_t;

typedef struct {
    const SongNote_t*    notes;
    uint16_t             note_count;
    const SongSection_t* sections;
    uint8_t              section_count;
} Song_t;


/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TFT_RES_Pin GPIO_PIN_15
#define TFT_RES_GPIO_Port GPIOA
#define TFT_DC_Pin GPIO_PIN_3
#define TFT_DC_GPIO_Port GPIOB
#define TFT_CS_Pin GPIO_PIN_6
#define TFT_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// Max number of notes to store in RAM
#define MAX_SONG_NOTES 300 
void Parse_Song_From_Buffer(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
