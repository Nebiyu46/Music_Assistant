#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "arm_math.h"
#include "song.h"

#define GAMEPLAY_LEAD_TIME_MS        1500
#define GAMEPLAY_AUDIO_WINDOW_MS     342
#define GAMEPLAY_GRACE_AFTER_PLAY_MS 200
#define GAMEPLAY_MAX_BRICKS          8
#define GAMEPLAY_RENDER_INTERVAL_MS  25

#define GAMEPLAY_BRICK_PENDING_COLOR CYAN
#define GAMEPLAY_BRICK_HIT_COLOR     GREEN
#define GAMEPLAY_BRICK_MISS_COLOR    RED
#define GAMEPLAY_MIN_BRICK_HEIGHT    4

void Gameplay_Start(const Song_t *song);
void Gameplay_Reset(void);
void Gameplay_SpawnBricks(const Song_t *song, uint8_t is_playing, uint8_t is_paused);
void Gameplay_PauseExpectedBrickIfDue(const Song_t *song, uint8_t is_playing, uint8_t is_paused);
void Gameplay_RenderBricks(const Song_t *song, uint8_t is_playing);
void Gameplay_ValidateBricks(const Song_t *song, uint8_t is_playing,
                             const float32_t *yin_peaks, const float32_t *fft_peaks, int n_peaks);

int32_t Gameplay_SongTimeMs(void);
void    Gameplay_PauseSong(void);
void    Gameplay_ResumeSong(void);
uint8_t Gameplay_IsPaused(void);

#endif /* GAMEPLAY_H */
