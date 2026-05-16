#ifndef SONG_H
#define SONG_H

#include <stdint.h>

#define MAX_SONG_NOTES        300
#define SONG_KEYS_PER_SECTION 8

typedef struct {
    uint16_t start_ms;
    uint16_t duration_ms;
    uint8_t  midi_note;
    uint8_t  key_idx;
} SongNote_t;

typedef struct {
    uint16_t first_note_idx;
    uint8_t  base_midi;
    const char *labels[SONG_KEYS_PER_SECTION];
} SongSection_t;

typedef struct {
    const SongNote_t    *notes;
    uint16_t             note_count;
    const SongSection_t *sections;
    uint8_t              section_count;
} Song_t;

#endif /* SONG_H */
