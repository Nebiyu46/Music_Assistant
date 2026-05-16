#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include <stdint.h>
#include "arm_math.h"

#define AUDIO_FFT_LENGTH 4096
#define AUDIO_SAMPLE_RATE_HZ 12000

void AudioCapture_Stop(void);
void AudioCapture_Start(void);
uint8_t AudioCapture_IsReady(void);
void AudioCapture_ClearReady(void);

extern volatile int audio_buffer_ready;
extern uint32_t adc_dma_buffer[AUDIO_FFT_LENGTH];
extern float32_t audio_input_buffer[AUDIO_FFT_LENGTH];
extern float32_t audio_yin_input_buffer[AUDIO_FFT_LENGTH];

void AudioCapture_FillFloatBuffers(void);

#endif /* AUDIO_CAPTURE_H */
