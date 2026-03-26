/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ST7735.h"
#include "GFX_FUNCTIONS.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern FontDef Font_7x10; // Standard font usually included in the library
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DISP_WIDTH 160
#define DISP_HEIGHT 128
#define NUM_KEYS 8
#define KEY_WIDTH  (DISP_WIDTH / NUM_KEYS)
#define KEY_HEIGHT 70  // Shorter keys so boxes have room to fall
#define KEYS_Y     (DISP_HEIGHT - KEY_HEIGHT) // Keys start at y=88
#define WHITE_KEY_COLOR WHITE
#define BLACK_KEY_COLOR BLACK
#define GREY_TEXT color565(128, 128, 128)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
// Song: Mary Had a Little Lamb (E, D, C, D, E, E, E)
// Mapped to white keys: C=0, D=1, E=2, F=3, G=4, A=5, B=6, C'=7
uint8_t demo_song[] = {2, 1, 0, 1, 2, 2, 2};
int current_note_idx = 0;
int falling_box_y = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void DrawPianoKeys(void) {
    // 1. Draw 8 White Keys at the bottom
    for(int i = 0; i < NUM_KEYS; i++) {
        int x = i * KEY_WIDTH;
        fillRect(x, KEYS_Y, KEY_WIDTH - 1, KEY_HEIGHT, WHITE_KEY_COLOR);

        // Label the keys in grey
        char* notes[] = {"C", "D", "E", "F", "G", "A", "B", "C"};
        // Note: If Font_7x10 causes an error, your font might be named differently.
        ST7735_WriteString(x + 5, KEYS_Y + 20, notes[i], Font_7x10, GREY_TEXT, WHITE_KEY_COLOR);
    }

    // 2. Draw the Black Keys on top
    for(int i = 0; i < NUM_KEYS - 1; i++) {
        if(i == 2 || i == 6) continue;

        int black_x = (i * KEY_WIDTH) + 15;
        int black_width = 10;
        int black_height = KEY_HEIGHT * 2 / 3;

        fillRect(black_x, KEYS_Y, black_width, black_height, BLACK_KEY_COLOR);
    }
}

void LightUpKey(uint8_t keyIndex, uint16_t color) {
    if(keyIndex >= NUM_KEYS) return;

    int x = keyIndex * KEY_WIDTH;
    fillRect(x, KEYS_Y, KEY_WIDTH - 1, KEY_HEIGHT, color);

    for(int i = 0; i < NUM_KEYS - 1; i++) {
        if(i == 2 || i == 6) continue;
        int black_x = (i * KEY_WIDTH) + 15;
        int black_width = 10;
        int black_height = KEY_HEIGHT * 2 / 3;
        fillRect(black_x, KEYS_Y, black_width, black_height, BLACK_KEY_COLOR);
    }
}

// Function to handle the game animation loop
void UpdateFallingNotes(void) {
    uint8_t target_key = demo_song[current_note_idx];
    int box_x = (target_key * KEY_WIDTH) + 2;
    int box_w = KEY_WIDTH - 5;
    int box_h = 10;
    int drop_speed = 5;

    // Erase the old box position
    fillRect(box_x, falling_box_y, box_w, box_h, BLACK);

    // Move box down
    falling_box_y += drop_speed;

    // Check if it hit the keys
    if (falling_box_y >= KEYS_Y - box_h) {
        // Hit! Light up the target key
        LightUpKey(target_key, GREEN);
        HAL_Delay(250); // Hold the lit key

        // Redraw keys normally
        DrawPianoKeys();

        // Reset box to top and move to next note
        falling_box_y = 0;
        current_note_idx++;

        if (current_note_idx >= sizeof(demo_song)) {
            current_note_idx = 0; // Loop the song
        }

        HAL_Delay(300); // Pause before next note falls
    } else {
        // Draw the new box position
        fillRect(box_x, falling_box_y, box_w, box_h, color565(0, 255, 255)); // Cyan box
        HAL_Delay(30); // Control the framerate of the fall
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */
  ST7735_Init(1);
  fillScreen(BLACK);
  DrawPianoKeys();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    UpdateFallingNotes();
    /* USER CODE END WHILE */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 90;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
