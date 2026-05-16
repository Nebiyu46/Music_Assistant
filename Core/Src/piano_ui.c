#include "piano_ui.h"

extern FontDef Font_7x10;

void PianoUI_DrawKeysSection(const SongSection_t *section)
{
    static const char *default_labels[PIANO_NUM_KEYS] = {
        "C", "D", "E", "F", "G", "A", "B", "C",
    };

    for (int i = 0; i < PIANO_NUM_KEYS; i++) {
        int x = i * PIANO_KEY_WIDTH;
        fillRect(x, PIANO_KEYS_Y, PIANO_KEY_WIDTH - 1, PIANO_KEY_HEIGHT, PIANO_WHITE_KEY_COLOR);

        const char *label = default_labels[i];
        if (section && section->labels[i]) {
            label = section->labels[i];
        }
        ST7735_WriteString((uint16_t)(x + 5), (uint16_t)(PIANO_KEYS_Y + 20),
                           (char *)label, Font_7x10, PIANO_GREY_TEXT, PIANO_WHITE_KEY_COLOR);
    }

    for (int i = 0; i < PIANO_NUM_KEYS - 1; i++) {
        if (i == 2 || i == 6) {
            continue;
        }
        int black_x = (i * PIANO_KEY_WIDTH) + 15;
        fillRect(black_x, PIANO_KEYS_Y, 10, (PIANO_KEY_HEIGHT * 2) / 3, PIANO_BLACK_KEY_COLOR);
    }
}

void PianoUI_DrawKeys(void)
{
    PianoUI_DrawKeysSection(NULL);
}

void PianoUI_LightUpKey(uint8_t key_index, uint16_t color)
{
    if (key_index >= PIANO_NUM_KEYS) {
        return;
    }

    int x = key_index * PIANO_KEY_WIDTH;
    fillRect(x, PIANO_KEYS_Y, PIANO_KEY_WIDTH - 1, PIANO_KEY_HEIGHT, color);

    for (int i = 0; i < PIANO_NUM_KEYS - 1; i++) {
        if (i == 2 || i == 6) {
            continue;
        }
        int black_x = (i * PIANO_KEY_WIDTH) + 15;
        fillRect(black_x, PIANO_KEYS_Y, 10, (PIANO_KEY_HEIGHT * 2) / 3, PIANO_BLACK_KEY_COLOR);
    }
}
