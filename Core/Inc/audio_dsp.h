#ifndef AUDIO_DSP_H
#define AUDIO_DSP_H

#include "arm_math.h"

#define AUDIO_DSP_PEAK_COUNT 6
#define AUDIO_USE_YIN        0
#define AUDIO_DEBUG_USB      1

void AudioDSP_Init(void);
void AudioDSP_ProcessFftPeaks(float32_t *output_array, int peak_count);
void AudioDSP_GetYinPeaks(float *input_buffer, int buffer_len, float32_t *output_array, int peak_count);
void AudioDSP_GetNoteName(float freq, char *output_buffer, int *midi_note);
void AudioDSP_SendMagnitudeToPc(float32_t *data_array, int length, int save_number);

#endif /* AUDIO_DSP_H */
