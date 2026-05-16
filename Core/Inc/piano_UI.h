#ifndef PIANO_UI_H
#define PIANO_UI_H

#include <stdint.h>
#include "song.h"
#include "ST7735.h"
#include "GFX_FUNCTIONS.h"

#define PIANO_DISP_WIDTH  160
#define PIANO_DISP_HEIGHT 128
#define PIANO_NUM_KEYS    8
#define PIANO_KEY_WIDTH   (PIANO_DISP_WIDTH / PIANO_NUM_KEYS)
#define PIANO_KEY_HEIGHT  70
#define PIANO_KEYS_Y      (PIANO_DISP_HEIGHT - PIANO_KEY_HEIGHT)

#define PIANO_WHITE_KEY_COLOR WHITE
#define PIANO_BLACK_KEY_COLOR BLACK
#define PIANO_GREY_TEXT       color565(128, 128, 128)

void PianoUI_DrawKeysSection(const SongSection_t *section);
void PianoUI_DrawKeys(void);
void PianoUI_LightUpKey(uint8_t key_index, uint16_t color);

#endif /* PIANO_UI_H */
