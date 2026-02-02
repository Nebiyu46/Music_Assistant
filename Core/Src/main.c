/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "arm_math.h"
#include "usbd_cdc_if.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

SongNote_t my_song[MAX_SONG_NOTES];
int song_len = 0;

// 2. The Raw Text Buffer (4KB is plenty for ~200 notes)
char usb_rx_buffer[4096]; 
int usb_rx_index = 0;

// 3. Flags
volatile uint8_t parsing_needed = 0;

arm_rfft_fast_instance_f32 fft_handler;
#define FFT_LENGTH 4096

const char* NOTE_NAMES[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// DMA buffer for ADC 
uint32_t adc_dma_buffer[FFT_LENGTH];
volatile int buffer_ready_flag = 0;

// 3. Define Buffers
// Input: Your audio samples go here
float32_t input_buffer[FFT_LENGTH]; 
float32_t first_400_for_Debugging[400];

// Output: The FFT writes raw complex data here. 
// Note: It needs the same size as input for the RFFT fast implementation
float32_t fft_output_buffer[FFT_LENGTH]; 

// Magnitude: The final "Volume per frequency" graph goes here
// Note: It is half the length because negative frequencies are removed
float32_t magnitude_buffer[FFT_LENGTH / 2];

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
void Get_Note_Name(float freq, char* output_buffer, int* midi_note);
/* USER CODE BEGIN PFP */

//uint32_t Mic_Read_Single_Sample(void);
void DSP_Init();
float Process_Audio();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  DSP_Init();
  /* USER CODE BEGIN 2 */
  
  //int indexx = 0;
//  int thirdd = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { 
    HAL_ADC_Start_DMA(&hadc1,adc_dma_buffer,FFT_LENGTH);
    HAL_TIM_Base_Start(&htim2);

    while (buffer_ready_flag == 0) {
      // Wait for DMA to complete
    }
    //copy dma buffer to input buffer
    //float32_t current;
    for (int i = 0; i < FFT_LENGTH; i++) {
      input_buffer[i] = (float)adc_dma_buffer[i]-2048; // Center around 0    
      if (i < 400) {
          first_400_for_Debugging[i] = input_buffer[i]; // For debugging
      }  
    }
    
    buffer_ready_flag = 0; // Clear flag for next round
    float freq = Process_Audio();
    char msg[50];
    int len = sprintf(msg, "Dominant Frequency: %.2f Hz\r\n", freq);
    CDC_Transmit_FS((uint8_t*)msg, len);
    char note_str[10];
    char usb_msg[64];
    int midi_num;

    Get_Note_Name(freq, note_str, &midi_num);
    if (midi_num != -1) {
        int leng = sprintf(usb_msg, "Freq: %.2f Hz | Note: %s | MIDI: %d\r\n", freq, note_str, midi_num);
        CDC_Transmit_FS((uint8_t*)usb_msg, leng);
        HAL_Delay(500);
    } 

      
    /*
    uint32_t mic_sample = Mic_Read_Single_Sample();
    float raw = (float)mic_sample - 2048.0f ;
    input_buffer[indexx] = raw; // Store in input buffer
    indexx++;
   if (indexx >= FFT_LENGTH) {
        indexx = 0; // Reset index after filling buffer
        float dominant_freq = Process_Audio();
        // Transmit dominant frequency over USB
        char msg[50];
        int len = sprintf(msg, "Dominant Frequency: %.2f Hz\r\n", dominant_freq);
        CDC_Transmit_FS((uint8_t*)msg, len);
        HAL_Delay(3000); //
    }
    */
      
    

    

    
    // Check if the USB transfer just finished
    /*
    if (parsing_needed == 1) {   // <--- START OF IF BLOCK
        
        parsing_needed = 0;      // Clear flag immediately
        Parse_Song_From_Buffer(); // Run the parser

        // --- NEW VERIFICATION CODE ---
        char msg[64];
        int song_len = 0;

        // 1. Print Header
        int len = sprintf(msg, "\r\n--- STM32 VERIFICATION ---\r\nParsed %d Notes:\r\n", song_len);
        CDC_Transmit_FS((uint8_t*)msg, len);
        HAL_Delay(20); // Give USB time to flush

        // 2. Loop through the array and print each note
        for (int i = 0; i < song_len; i++) {
            // Format: "#0: T=0, N=60, D=500"
            song_len++;
            len = sprintf(msg, "#%d: T=%lu, N=%d, D=%lu\r\n", 
                          i, 
                          my_song[i].start_ms, 
                          my_song[i].midi_note, 
                          my_song[i].duration_ms);
            
            CDC_Transmit_FS((uint8_t*)msg, len);
            
            
            // CRITICAL: Short delay to prevent crashing the terminal
            HAL_Delay(5); 
        }

        // 3. Print Footer
        CDC_Transmit_FS((uint8_t*)"--- END ---\r\n", 13);
        
    }
    for (int i = 0; i < song_len; i++) {
        // Here you can implement code to play the song using the parsed notes
        // For example, you might toggle GPIO pins to generate sound or send MIDI messages
        if (my_song[i].midi_note < 60) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2); // Toggle pin for note 60
            HAL_Delay(my_song[i].duration_ms);
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2); // Toggle pin off
        }
        else if (my_song[i].midi_note >= 60) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Toggle pin for note 62
            HAL_Delay(my_song[i].duration_ms);
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Toggle pin off
        }
    }
    */
      
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 8000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA2 PA5 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/*
uint32_t Mic_Read_Single_Sample(void){
    uint32_t raw_value = 0;

    // 1. Start the ADC hardware
    HAL_ADC_Start(&hadc1);

    // 2. Wait for the conversion to finish (Timeout: 1ms)
    if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK)
    {
        // 3. Read the value register
        raw_value = HAL_ADC_GetValue(&hadc1);
    }

    // 4. Stop to save power (optional in loop)
    HAL_ADC_Stop(&hadc1);

    return raw_value;
}
*/

/* USER CODE BEGIN 4 */
// This function runs every time USB data arrives (Interrupt context)
void USB_Data_Receiver(uint8_t* Buf, uint32_t Len) {
    
    // Check for overflow
    if (usb_rx_index + Len >= 4096) {
        usb_rx_index = 0; // Reset on overflow safety
    }

    // 1. Copy incoming data to our big buffer
    memcpy(&usb_rx_buffer[usb_rx_index], Buf, Len);
    usb_rx_buffer[usb_rx_index + Len] = '\0'; // Null terminate for string safety
    
    // 2. Check for Commands
    // Check for "START" (Reset everything)
    if (strstr(&usb_rx_buffer[usb_rx_index], "START") != NULL) {
        usb_rx_index = 0; // Clear buffer
        memset(usb_rx_buffer, 0, 4096);
        return;
    }

    // Check for "END" (Trigger Parsing)
    if (strstr(&usb_rx_buffer[usb_rx_index], "END") != NULL) {
        parsing_needed = 1; // Tell main loop to do the work
    }

    // Advance index
    usb_rx_index += Len;
}

// Helper to actually convert text to numbers
void Parse_Song_From_Buffer(void) {
    char *line;
    char *rest = usb_rx_buffer;
    song_len = 0;

    printf("Parsing Song...\r\n"); // Debug info

    // Split buffer by newlines "\n"
    while ((line = strtok_r(rest, "\n", &rest))) {
        
        // Skip commands
        if (strstr(line, "START") || strstr(line, "END")) continue;

        // Parse: "0,60,500" -> int, int, int
        uint32_t start, dur;
        int note;
        
        if (sscanf(line, "%lu,%d,%lu", &start, &note, &dur) == 3) {
            if (song_len < MAX_SONG_NOTES) {
                my_song[song_len].start_ms = start;
                my_song[song_len].midi_note = (uint8_t)note;
                my_song[song_len].duration_ms = dur;
                song_len++;
            }
        }
    }
    printf("Song Loaded! Total Notes: %d\r\n", song_len);
}



void DSP_Init() {
    // Initialize any DSP-related structures or settings here
    arm_rfft_fast_init_f32(&fft_handler, FFT_LENGTH);
}

float Process_Audio() {
    // Step A: Run the FFT
    // Arguments: &manager, input_array, output_array, inverse_flag(0)
    arm_rfft_fast_f32(&fft_handler, input_buffer, fft_output_buffer, 0);
    float32_t twoxed[FFT_LENGTH/4];
    float32_t threeexed[(int)(FFT_LENGTH/2)/3];
    

    // Step B: Calculate Magnitude (Sqrt(Real^2 + Imag^2))
    // We need this because the FFT output is complex (Real, Imag, Real, Imag...)
    arm_cmplx_mag_f32(fft_output_buffer, magnitude_buffer, FFT_LENGTH / 2);

    // Step C: Find the Dominant Frequency (Pitch)
    float32_t max_value;
    uint32_t  max_index;
    for (int i = 0; i < FFT_LENGTH / 2; i++) {
        if(i < (FFT_LENGTH / 2)/4) {
            magnitude_buffer[i] *= magnitude_buffer[i*2]*magnitude_buffer[i*4]*magnitude_buffer[i*3];
            continue;
        }
        if (i < (FFT_LENGTH / 2)/3) {
            magnitude_buffer[i] *= magnitude_buffer[i*3]*magnitude_buffer[i*2];
            continue;
        }
        if (i < FFT_LENGTH / 4) {
            magnitude_buffer[i] *= magnitude_buffer[i*2];
        }
                
    }
    // Finds the loudest frequency bin
    arm_max_f32(&magnitude_buffer[1], (FFT_LENGTH / 2)-1 , &max_value, &max_index);
    // Calculate actual Frequency in Hz
    // Formula: Index * Sampling_Rate / FFT_Length
    float freq_hz = (float)(max_index+1) * 12000.0f / FFT_LENGTH;

    return freq_hz;
}


//DMA Complete Callback
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  if (hadc->Instance == ADC1) {
        // Stop the Timer and ADC so we can process without overwriting
        HAL_TIM_Base_Stop(&htim2);
        HAL_ADC_Stop_DMA(&hadc1); 
        buffer_ready_flag = 1; 
    }
}

void Get_Note_Name(float freq, char* output_buffer, int* midi_note) {
    // 1. Handle Silence / Noise
    if (freq < 20.0f || freq > 4200.0f) {
        sprintf(output_buffer, "---");
        *midi_note = -1;
        return;
    }

    // 2. Calculate MIDI Note Number
    // Formula: n = 12 * log2(freq / 440) + 69
    // We use log(x) / log(2) because standard C might not have log2f optimized
    float note_num_float = 12.0f * (logf(freq / 440.0f) / logf(2.0f)) + 69.0f;
    
    // 3. Round to nearest integer (Standard Rounding)
    int note_num = (int)(note_num_float + 0.5f);
    *midi_note = note_num;
    

    // 4. Calculate Octave and Note Index
    // MIDI 0 is C-1. MIDI 12 is C0. MIDI 60 is C4 (Middle C).
    int octave = (note_num / 12) - 1;
    int note_index = note_num % 12;

    // 5. Handle bounds (Optional safety)
    if (note_index < 0) note_index = 0; // Prevent negative array index
    
    // 6. Write to buffer (e.g., "C" + "4" = "C4")
    sprintf(output_buffer, "%s%d", NOTE_NAMES[note_index], octave);

}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
