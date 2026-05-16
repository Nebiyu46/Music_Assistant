#include "gameplay.h"
#include "audio_dsp.h"
#include "piano_ui.h"
#include "GFX_FUNCTIONS.h"

typedef struct {
    uint8_t  active;
    uint16_t note_idx;
    int16_t  prev_y_top;
    int16_t  prev_height;
    uint8_t  hit_status;
} ActiveBrick_t;

static ActiveBrick_t bricks[GAMEPLAY_MAX_BRICKS];

static uint32_t song_origin_ms;
static uint32_t paused_total_ms;
static uint32_t pause_start_ms;
static uint8_t  is_paused;
static uint8_t  song_started;

static uint16_t next_spawn_idx;
static uint16_t expected_idx;
static uint8_t  current_section;
static uint8_t  last_drawn_section = 0xFF;

void Gameplay_Reset(void)
{
    song_origin_ms = 0;
    paused_total_ms = 0;
    pause_start_ms = 0;
    is_paused = 0;
    song_started = 0;
    next_spawn_idx = 0;
    expected_idx = 0;
    current_section = 0;
    last_drawn_section = 0xFF;

    for (int i = 0; i < GAMEPLAY_MAX_BRICKS; i++) {
        bricks[i].active = 0;
        bricks[i].note_idx = 0;
        bricks[i].prev_y_top = 0;
        bricks[i].prev_height = 0;
        bricks[i].hit_status = 0;
    }
}

void Gameplay_Start(const Song_t *song)
{
    (void)song;
    song_origin_ms = HAL_GetTick();
    paused_total_ms = 0;
    pause_start_ms = 0;
    is_paused = 0;
    song_started = 1;
    next_spawn_idx = 0;
    expected_idx = 0;
    current_section = 0;
    last_drawn_section = 0;

    for (int i = 0; i < GAMEPLAY_MAX_BRICKS; i++) {
        bricks[i].active = 0;
        bricks[i].note_idx = 0;
        bricks[i].prev_y_top = 0;
        bricks[i].prev_height = 0;
        bricks[i].hit_status = 0;
    }
}

void Gameplay_PauseSong(void)
{
    if (is_paused) {
        return;
    }
    pause_start_ms = HAL_GetTick();
    is_paused = 1;
}

void Gameplay_ResumeSong(void)
{
    if (!is_paused) {
        return;
    }
    paused_total_ms += (HAL_GetTick() - pause_start_ms);
    is_paused = 0;
}

uint8_t Gameplay_IsPaused(void)
{
    return is_paused;
}

int32_t Gameplay_SongTimeMs(void)
{
    uint32_t now;
    uint32_t reference;

    if (!song_started) {
        return 0;
    }

    now = HAL_GetTick();
    reference = is_paused ? pause_start_ms : now;
    return (int32_t)(reference - song_origin_ms - paused_total_ms);
}

void Gameplay_SpawnBricks(const Song_t *song, uint8_t is_playing, uint8_t is_paused_now)
{
    int32_t t_song;
    int slot;

    if (!is_playing || song == NULL || song->note_count == 0U || !song_started || is_paused_now) {
        return;
    }

    t_song = Gameplay_SongTimeMs();

    while (next_spawn_idx < song->note_count) {
        const SongNote_t *n = &song->notes[next_spawn_idx];
        int32_t spawn_at = (int32_t)n->start_ms - GAMEPLAY_LEAD_TIME_MS;

        if (t_song < spawn_at) {
            break;
        }

        slot = -1;
        for (int i = 0; i < GAMEPLAY_MAX_BRICKS; i++) {
            if (!bricks[i].active) {
                slot = i;
                break;
            }
        }
        if (slot < 0) {
            break;
        }

        bricks[slot].active = 1;
        bricks[slot].note_idx = next_spawn_idx;
        bricks[slot].prev_y_top = 0;
        bricks[slot].prev_height = 0;
        bricks[slot].hit_status = 0;
        next_spawn_idx++;
    }
}

void Gameplay_PauseExpectedBrickIfDue(const Song_t *song, uint8_t is_playing, uint8_t is_paused_now)
{
    int32_t t_song;

    if (!is_playing || song == NULL || !song_started || is_paused_now) {
        return;
    }

    t_song = Gameplay_SongTimeMs();

    for (int i = 0; i < GAMEPLAY_MAX_BRICKS; i++) {
        const SongNote_t *n;

        if (!bricks[i].active) {
            continue;
        }
        if (bricks[i].note_idx != expected_idx) {
            continue;
        }
        if (bricks[i].hit_status != 0) {
            return;
        }

        n = &song->notes[bricks[i].note_idx];
        if (t_song >= (int32_t)n->start_ms) {
            bricks[i].hit_status = 2;
            Gameplay_PauseSong();
        }
        return;
    }
}

void Gameplay_RenderBricks(const Song_t *song, uint8_t is_playing)
{
    static uint32_t last_render_tick;
    uint32_t now;
    int32_t t_song;

    if (!is_playing || song == NULL) {
        return;
    }

    now = HAL_GetTick();
    if ((now - last_render_tick) < GAMEPLAY_RENDER_INTERVAL_MS) {
        return;
    }
    last_render_tick = now;

    t_song = Gameplay_SongTimeMs();

    for (int i = 0; i < GAMEPLAY_MAX_BRICKS; i++) {
        const SongNote_t *n;
        int32_t spawn_at;
        int32_t elapsed;
        int32_t y_bottom;
        int32_t height;
        int32_t y_top;
        int is_stuck;
        int px;
        int pw;
        int32_t draw_top;
        int32_t draw_h;
        uint16_t color;

        if (!bricks[i].active) {
            continue;
        }

        n = &song->notes[bricks[i].note_idx];
        spawn_at = (int32_t)n->start_ms - GAMEPLAY_LEAD_TIME_MS;
        elapsed = t_song - spawn_at;
        if (elapsed < 0) {
            elapsed = 0;
        }

        y_bottom = (elapsed * PIANO_KEYS_Y) / GAMEPLAY_LEAD_TIME_MS;
        height = ((int32_t)n->duration_ms * PIANO_KEYS_Y) / GAMEPLAY_LEAD_TIME_MS;
        if (height < GAMEPLAY_MIN_BRICK_HEIGHT) {
            height = GAMEPLAY_MIN_BRICK_HEIGHT;
        }
        y_top = y_bottom - height;

        is_stuck = is_paused && (bricks[i].note_idx == expected_idx) && (bricks[i].hit_status == 2);
        if (is_stuck) {
            y_top = PIANO_KEYS_Y - height;
            y_bottom = PIANO_KEYS_Y;
        }

        px = n->key_idx * PIANO_KEY_WIDTH + 2;
        pw = PIANO_KEY_WIDTH - 5;

        if (bricks[i].prev_height > 0) {
            fillRect(px, bricks[i].prev_y_top, pw, bricks[i].prev_height, BLACK);
            bricks[i].prev_height = 0;
        }

        if (y_top >= PIANO_KEYS_Y) {
            bricks[i].active = 0;
            continue;
        }

        draw_top = y_top;
        draw_h = height;
        if (draw_top < 0) {
            draw_h += draw_top;
            draw_top = 0;
        }
        if (draw_top + draw_h > PIANO_KEYS_Y) {
            draw_h = PIANO_KEYS_Y - draw_top;
        }
        if (draw_h <= 0) {
            continue;
        }

        switch (bricks[i].hit_status) {
        case 1:
            color = GAMEPLAY_BRICK_HIT_COLOR;
            break;
        case 2:
            color = GAMEPLAY_BRICK_MISS_COLOR;
            break;
        default:
            color = GAMEPLAY_BRICK_PENDING_COLOR;
            break;
        }

        fillRect(px, draw_top, pw, draw_h, color);
        bricks[i].prev_y_top = (int16_t)draw_top;
        bricks[i].prev_height = (int16_t)draw_h;
    }
}

void Gameplay_ValidateBricks(const Song_t *song, uint8_t is_playing,
                             const float32_t *yin_peaks, const float32_t *fft_peaks, int n_peaks)
{
    int candidates[16];
    int n_candidates = 0;
    char dummy[10];
    int midi;
    int32_t t_song;
    int32_t audio_end;
    int32_t audio_start;

    if (!is_playing || song == NULL || !song_started) {
        return;
    }

    for (int i = 0; i < n_peaks; i++) {
        if (yin_peaks) {
            AudioDSP_GetNoteName(yin_peaks[i], dummy, &midi);
            if (midi >= 0 && n_candidates < 16) {
                candidates[n_candidates++] = midi;
            }
        }
        if (fft_peaks) {
            AudioDSP_GetNoteName(fft_peaks[i], dummy, &midi);
            if (midi >= 0 && n_candidates < 16) {
                candidates[n_candidates++] = midi;
            }
        }
    }

    t_song = Gameplay_SongTimeMs();
    audio_end = t_song;
    audio_start = t_song - GAMEPLAY_AUDIO_WINDOW_MS;

    for (int i = 0; i < GAMEPLAY_MAX_BRICKS; i++) {
        const SongNote_t *n;
        int32_t play_start;
        int32_t play_end;
        int is_stuck;
        int audio_before;
        int audio_after;

        if (!bricks[i].active) {
            continue;
        }
        if (bricks[i].hit_status == 1) {
            continue;
        }

        n = &song->notes[bricks[i].note_idx];
        play_start = (int32_t)n->start_ms;
        play_end = play_start + (int32_t)n->duration_ms + GAMEPLAY_GRACE_AFTER_PLAY_MS;

        is_stuck = (bricks[i].hit_status == 2);
        audio_before = (audio_end < play_start);
        audio_after = (audio_start > play_end);

        if (audio_before && !is_stuck) {
            continue;
        }
        if (audio_after && !is_stuck) {
            bricks[i].hit_status = 2;
            if (bricks[i].note_idx == expected_idx) {
                Gameplay_PauseSong();
            }
            continue;
        }

        for (int c = 0; c < n_candidates; c++) {
            if (candidates[c] == n->midi_note) {
                bricks[i].hit_status = 1;
                break;
            }
        }
    }

    while (expected_idx < song->note_count) {
        int found = -1;

        for (int i = 0; i < GAMEPLAY_MAX_BRICKS; i++) {
            if (bricks[i].active && bricks[i].note_idx == expected_idx) {
                found = i;
                break;
            }
        }
        if (found < 0) {
            break;
        }
        if (bricks[found].hit_status != 1) {
            break;
        }
        expected_idx++;
        if (is_paused) {
            Gameplay_ResumeSong();
        }
    }

    while ((current_section + 1) < song->section_count &&
           expected_idx >= song->sections[current_section + 1].first_note_idx) {
        current_section++;
        if (current_section != last_drawn_section) {
            PianoUI_DrawKeysSection(&song->sections[current_section]);
            last_drawn_section = current_section;
        }
    }
}
