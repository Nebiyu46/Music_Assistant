#include "audio_capture.h"
#include "board.h"

volatile int audio_buffer_ready;
uint32_t adc_dma_buffer[AUDIO_FFT_LENGTH];
float32_t audio_input_buffer[AUDIO_FFT_LENGTH];
float32_t audio_yin_input_buffer[AUDIO_FFT_LENGTH];

void AudioCapture_Stop(void)
{
    HAL_TIM_Base_Stop(&htim2);
    HAL_ADC_Stop_DMA(&hadc1);
    audio_buffer_ready = 0;
}

void AudioCapture_Start(void)
{
    audio_buffer_ready = 0;
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_TIM_Base_Stop(&htim2);
    HAL_ADC_Start_DMA(&hadc1, adc_dma_buffer, AUDIO_FFT_LENGTH);
    HAL_TIM_Base_Start(&htim2);
}

uint8_t AudioCapture_IsReady(void)
{
    return (uint8_t)(audio_buffer_ready != 0);
}

void AudioCapture_ClearReady(void)
{
    audio_buffer_ready = 0;
}

void AudioCapture_FillFloatBuffers(void)
{
    for (int i = 0; i < AUDIO_FFT_LENGTH; i++) {
        audio_input_buffer[i] = (float)adc_dma_buffer[i] - 2048.0f;
        audio_yin_input_buffer[i] = ((float)adc_dma_buffer[i] - 2048.0f) / 2048.0f;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        HAL_TIM_Base_Stop(&htim2);
        HAL_ADC_Stop_DMA(&hadc1);
        audio_buffer_ready = 1;
    }
}
