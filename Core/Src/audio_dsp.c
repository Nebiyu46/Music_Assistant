#include "audio_dsp.h"
#include "audio_capture.h"
#include "usbd_cdc_if.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define FFT_PEAK_THRESHOLD 8000.0f
#define YIN_THRESHOLD      0.15f
#define YIN_BUFFER_SIZE    (AUDIO_FFT_LENGTH / 2)

static const char *note_names[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
};

static arm_rfft_fast_instance_f32 fft_handler;
static float yin_buffer[YIN_BUFFER_SIZE];
static float32_t fft_output_buffer[AUDIO_FFT_LENGTH];
static float32_t magnitude_buffer[AUDIO_FFT_LENGTH / 2];
static int current_save_number;

void AudioDSP_Init(void)
{
    arm_rfft_fast_init_f32(&fft_handler, AUDIO_FFT_LENGTH);
}

void AudioDSP_ProcessFftPeaks(float32_t *output_array, int peak_count)
{
    int len = AUDIO_FFT_LENGTH / 2;

    arm_rfft_fast_f32(&fft_handler, audio_input_buffer, fft_output_buffer, 0);
    arm_cmplx_mag_f32(fft_output_buffer, magnitude_buffer, len);

    for (int i = 0; i < peak_count; i++) {
        float32_t max_value;
        uint32_t max_index_raw;

        arm_max_f32(&magnitude_buffer[1], len - 1, &max_value, &max_index_raw);
        if (max_value < FFT_PEAK_THRESHOLD) {
            output_array[i] = 0.0f;
        } else {
            output_array[i] =
                (float)(max_index_raw + 1) * AUDIO_SAMPLE_RATE_HZ / (float)AUDIO_FFT_LENGTH;
        }

        for (int j = -1; j < 3; j++) {
            int idx = (int)max_index_raw + j;
            if (idx >= 0 && idx < len) {
                magnitude_buffer[idx] = 0.0f;
            }
        }
    }
}

void AudioDSP_GetYinPeaks(float *input_buffer, int buffer_len, float32_t *output_array, int peak_count)
{
    int tau;
    int max_tau = buffer_len / 2;

    if (max_tau > YIN_BUFFER_SIZE) {
        max_tau = YIN_BUFFER_SIZE;
    }

    float fixed_energy = 0.0f;
    arm_dot_prod_f32(input_buffer, input_buffer, max_tau, &fixed_energy);

    float sliding_energy = fixed_energy;
    int candidates[AUDIO_DSP_PEAK_COUNT / 2];
    int num_candidates = 0;

    for (tau = 0; tau < max_tau; tau++) {
        float correlation = 0.0f;
        arm_dot_prod_f32(&input_buffer[0], &input_buffer[tau], max_tau, &correlation);

        yin_buffer[tau] = fixed_energy + sliding_energy - (2.0f * correlation);

        float sample_out = input_buffer[tau];
        float sample_in = input_buffer[tau + max_tau];
        sliding_energy = sliding_energy - (sample_out * sample_out) + (sample_in * sample_in);
    }

    yin_buffer[0] = 1.0f;
    float running_sum = 0.0f;

    for (tau = 1; tau < max_tau; tau++) {
        running_sum += yin_buffer[tau];
        if (running_sum == 0.0f) {
            yin_buffer[tau] = 1.0f;
        } else {
            yin_buffer[tau] *= (float)tau / running_sum;
        }
    }

    for (tau = 2; tau < max_tau; tau++) {
        if (yin_buffer[tau] < YIN_THRESHOLD) {
            while (tau + 1 < max_tau && yin_buffer[tau + 1] < yin_buffer[tau]) {
                tau++;
            }
            if (num_candidates < peak_count) {
                candidates[num_candidates++] = tau;
            }
            tau++;
        }
    }

    if (num_candidates == 0) {
        for (int i = 0; i < peak_count; i++) {
            output_array[i] = 0.0f;
        }
        return;
    }

    for (int i = 0; i < peak_count; i++) {
        if (i >= num_candidates) {
            output_array[i] = 0.0f;
            continue;
        }

        int best_tau = candidates[i];
        float refined_tau = (float)best_tau;

        if (best_tau > 0 && best_tau < max_tau - 1) {
            float s0 = yin_buffer[best_tau - 1];
            float s1 = yin_buffer[best_tau];
            float s2 = yin_buffer[best_tau + 1];
            float adjustment = (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
            refined_tau += adjustment;
        }

        output_array[i] = (float32_t)AUDIO_SAMPLE_RATE_HZ / refined_tau;
    }
}

void AudioDSP_GetNoteName(float freq, char *output_buffer, int *midi_note)
{
    if (freq < 20.0f || freq > 4200.0f) {
        *midi_note = -1;
        sprintf(output_buffer, "Z0");
        return;
    }

    float note_num_float = 12.0f * (logf(freq / 440.0f) / logf(2.0f)) + 69.0f;
    int note_num = (int)(note_num_float + 0.5f);
    int octave;
    int note_index;

    *midi_note = note_num;
    octave = (note_num / 12) - 1;
    note_index = note_num % 12;

    if (note_index < 0) {
        note_index = 0;
    }

    sprintf(output_buffer, "%s%d", note_names[note_index], octave);
}

void AudioDSP_SendMagnitudeToPc(float32_t *data_array, int length, int save_number)
{
    char msg_buffer[64];
    int len;

    len = sprintf(msg_buffer, "\r\n\r\n=== BATCH %d ===\r\n", save_number);
    CDC_Transmit_FS((uint8_t *)msg_buffer, (uint16_t)len);
    HAL_Delay(5);

    for (int i = 0; i < length; i++) {
        len = sprintf(msg_buffer, "%d, %.2f\r\n", i, data_array[i]);
        CDC_Transmit_FS((uint8_t *)msg_buffer, (uint16_t)len);
        HAL_Delay(2);
    }

    len = sprintf(msg_buffer, "=== END ===\r\n");
    CDC_Transmit_FS((uint8_t *)msg_buffer, (uint16_t)len);

    current_save_number = save_number + 1;
}