#include "app_ui.h"
#include "main.h"
#include "ST7735.h"

#include <math.h>
#include <stdio.h>

extern FontDef Font_7x10;
extern FontDef Font_11x18;

#define HOME_COLOR_BLACK   0x0000u
#define HOME_COLOR_WHITE   0xFFFFu
#define HOME_COLOR_CYAN    0x07FFu
#define HOME_COLOR_MAGENTA 0xF81Fu
#define HOME_COLOR_DARKBG  0x0801u
#define HOME_ANIM_W        160
#define HOME_ANIM_H        25

static uint16_t home_anim_buf[HOME_ANIM_W * HOME_ANIM_H];
static uint16_t home_scroll_frame;
static int8_t home_smooth_curve[160];

static void draw_pixel_home_buf(int16_t x, int16_t y, uint16_t color)
{
    if (x >= 0 && x < HOME_ANIM_W && y >= 0 && y < HOME_ANIM_H) {
        home_anim_buf[(uint16_t)y * HOME_ANIM_W + (uint16_t)x] =
            (uint16_t)((color >> 8) | (color << 8));
    }
}

static void draw_note_home_buf(int16_t x, int16_t y, uint16_t color)
{
    for (int dx = 0; dx < 5; dx++) {
        for (int dy = 0; dy < 4; dy++) {
            draw_pixel_home_buf((int16_t)(x + dx + 1), (int16_t)(y + 7 + dy), color);
        }
    }
    for (int dy = 0; dy < 8; dy++) {
        draw_pixel_home_buf((int16_t)(x + 5), (int16_t)(y + dy), color);
        draw_pixel_home_buf((int16_t)(x + 6), (int16_t)(y + dy), color);
    }
    for (int dx = 0; dx < 7; dx++) {
        draw_pixel_home_buf((int16_t)(x + 5 + dx), y, color);
        draw_pixel_home_buf((int16_t)(x + 5 + dx), (int16_t)(y + 1), color);
    }
}

static uint8_t poll_button_edge(GPIO_TypeDef *port, uint16_t pin,
                                uint8_t *stable_released, uint8_t *last_raw_released,
                                uint32_t *change_tick, uint8_t *prev_stable_released)
{
    uint8_t raw_released = (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET);
    uint32_t now = HAL_GetTick();

    if (raw_released != *last_raw_released) {
        *change_tick = now;
        *last_raw_released = raw_released;
    }
    if ((now - *change_tick) >= 30U) {
        *stable_released = raw_released;
    }

    uint8_t pressed = (*prev_stable_released && !*stable_released);
    *prev_stable_released = *stable_released;
    return pressed;
}

static void draw_screen_header(const char *title)
{
    ST7735_FillScreen(HOME_COLOR_BLACK);
    ST7735_FillRectangle(0, 0, 160, 3, HOME_COLOR_MAGENTA);
    ST7735_FillRectangle(0, 28, 160, 1, HOME_COLOR_CYAN);
    ST7735_WriteString(8, 10, (char *)title, Font_11x18, HOME_COLOR_CYAN, HOME_COLOR_BLACK);
    ST7735_FillRectangle(8, 30, 144, 1, HOME_COLOR_MAGENTA);
}

void AppUI_InitHome(void)
{
    int bk_pattern[] = {1, 1, 0, 1, 1, 1, 0};

    for (int i = 0; i < 160; i++) {
        home_smooth_curve[i] = (int8_t)(sinf((float)i * 2.0f * 3.14159265f / 160.0f) * 3.0f);
    }
    home_scroll_frame = 0;

    ST7735_FillScreen(HOME_COLOR_BLACK);
    ST7735_FillRectangle(0, 0, 160, 3, HOME_COLOR_MAGENTA);
    ST7735_FillRectangle(0, 28, 160, 1, HOME_COLOR_CYAN);
    ST7735_FillRectangle(0, 29, 160, 42, HOME_COLOR_BLACK);
    ST7735_WriteString(53, 33, "PIANO", Font_11x18, HOME_COLOR_MAGENTA, HOME_COLOR_BLACK);
    ST7735_WriteString(52, 32, "PIANO", Font_11x18, HOME_COLOR_CYAN, HOME_COLOR_BLACK);
    ST7735_FillRectangle(40, 53, 80, 1, HOME_COLOR_CYAN);
    ST7735_WriteString(55, 57, "TRAINER", Font_7x10, HOME_COLOR_CYAN, HOME_COLOR_BLACK);
    ST7735_FillRectangle(0, 71, 160, 1, HOME_COLOR_CYAN);
    ST7735_FillRectangle(0, 72, 160, 2, HOME_COLOR_MAGENTA);
    ST7735_FillRectangle(0, 74, 160, 23, HOME_COLOR_DARKBG);
    for (int i = 0; i < 10; i++) {
        ST7735_FillRectangle((uint16_t)(10 + (i * 15)), (uint16_t)((i % 2 == 0) ? 80 : 86), 2, 2,
                             (uint16_t)((i % 2 == 0) ? HOME_COLOR_MAGENTA : HOME_COLOR_CYAN));
    }
    ST7735_WriteString(27, 81, "> PRESS START <", Font_7x10, HOME_COLOR_MAGENTA, HOME_COLOR_DARKBG);
    ST7735_FillRectangle(0, 97, 160, 2, HOME_COLOR_MAGENTA);
    ST7735_FillRectangle(0, 99, 160, 1, HOME_COLOR_CYAN);
    ST7735_FillRectangle(0, 100, 160, 28, HOME_COLOR_WHITE);
    for (int i = 1; i < 20; i++) {
        ST7735_FillRectangle((uint16_t)(i * 8), 100, 1, 28, HOME_COLOR_BLACK);
    }
    for (int i = 0; i < 19; i++) {
        if (bk_pattern[i % 7] == 1) {
            ST7735_FillRectangle((uint16_t)((i * 8) + 5), 100, 5, 16, HOME_COLOR_BLACK);
        }
    }
    ST7735_FillRectangle(0, 100, 160, 1, HOME_COLOR_BLACK);
}

void AppUI_AnimateHome(void)
{
    for (int i = 0; i < HOME_ANIM_W * HOME_ANIM_H; i++) {
        home_anim_buf[i] = (uint16_t)((HOME_COLOR_DARKBG >> 8) | (HOME_COLOR_DARKBG << 8));
    }

    home_scroll_frame++;
    for (int x = 0; x < HOME_ANIM_W; x++) {
        int idx = (x + (int)home_scroll_frame) % 160;
        int y_offset = home_smooth_curve[idx];
        for (int line = 0; line < 3; line++) {
            int base_y = 8 + (line * 6);
            draw_pixel_home_buf((int16_t)x, (int16_t)(base_y + y_offset), HOME_COLOR_CYAN);
        }
    }

    for (int i = 0; i < 5; i++) {
        int x_pos = (int)((i * 40 - (int)home_scroll_frame) % 200);
        int curve_idx;
        int y_offset;
        uint16_t note_color;
        int note_y;

        if (x_pos < 0) {
            x_pos += 200;
        }
        x_pos -= 20;
        curve_idx = (x_pos + (int)home_scroll_frame) % 160;
        if (curve_idx < 0) {
            curve_idx += 160;
        }
        y_offset = home_smooth_curve[curve_idx];
        note_color = (uint16_t)((i % 2 == 0) ? HOME_COLOR_MAGENTA : HOME_COLOR_WHITE);
        note_y = (i % 3) * 6;
        draw_note_home_buf((int16_t)x_pos, (int16_t)(note_y + y_offset), note_color);
    }

    ST7735_DrawImage(0, 3, HOME_ANIM_W, HOME_ANIM_H, home_anim_buf);
}

uint8_t AppUI_PollOkPressed(void)
{
    static uint8_t stable = 1;
    static uint8_t last_raw = 1;
    static uint32_t change_tick = 0;
    static uint8_t prev_stable = 1;

    return poll_button_edge(BTN_OK_GPIO_Port, BTN_OK_Pin,
                           &stable, &last_raw, &change_tick, &prev_stable);
}

uint8_t AppUI_PollNavPressed(void)
{
    static uint8_t stable = 1;
    static uint8_t last_raw = 1;
    static uint32_t change_tick = 0;
    static uint8_t prev_stable = 1;

    return poll_button_edge(BTN_NAV_GPIO_Port, BTN_NAV_Pin,
                           &stable, &last_raw, &change_tick, &prev_stable);
}

void AppUI_DrawMenu(int8_t selection)
{
    static const char *labels[2] = {"Select Song", "Upload Song"};

    draw_screen_header("MENU");

    for (int i = 0; i < 2; i++) {
        int16_t y = (int16_t)(52 + (i * 22));
        uint16_t fg = HOME_COLOR_WHITE;
        uint16_t bg = HOME_COLOR_BLACK;

        if (i == selection) {
            ST7735_FillRectangle(6, (uint16_t)(y - 3), 148, 18, HOME_COLOR_DARKBG);
            fg = HOME_COLOR_MAGENTA;
            bg = HOME_COLOR_DARKBG;
            ST7735_WriteString(12, (uint16_t)y, ">", Font_7x10, HOME_COLOR_CYAN, bg);
        }

        ST7735_WriteString(24, (uint16_t)y, (char *)labels[i], Font_7x10, fg, bg);
    }

    ST7735_WriteString(8, 110, "PA2: move  PA1: OK", Font_7x10, HOME_COLOR_CYAN, HOME_COLOR_BLACK);
}

void AppUI_DrawSongList(int8_t selection, const char *const *titles, int count)
{
    draw_screen_header("SONGS");

    if (count == 0) {
        ST7735_WriteString(8,  65, "No songs on SD card", Font_7x10,
                           HOME_COLOR_MAGENTA, HOME_COLOR_BLACK);
        ST7735_WriteString(8,  80, "Upload a song first", Font_7x10,
                           HOME_COLOR_WHITE,   HOME_COLOR_BLACK);
        ST7735_WriteString(8, 110, "PA1: back",           Font_7x10,
                           HOME_COLOR_CYAN,    HOME_COLOR_BLACK);
        return;
    }

    if (selection >= count) {
        selection = 0;
    }

    for (int i = 0; i < count; i++) {
        int16_t  y  = (int16_t)(52 + (i * 22));
        uint16_t fg = HOME_COLOR_WHITE;
        uint16_t bg = HOME_COLOR_BLACK;

        if (i == selection) {
            ST7735_FillRectangle(6, (uint16_t)(y - 3), 148, 18, HOME_COLOR_DARKBG);
            fg = HOME_COLOR_MAGENTA;
            bg = HOME_COLOR_DARKBG;
            ST7735_WriteString(12, (uint16_t)y, ">", Font_7x10, HOME_COLOR_CYAN, bg);
        }

        ST7735_WriteString(24, (uint16_t)y, (char *)titles[i], Font_7x10, fg, bg);
    }

    ST7735_WriteString(8, 110, "PA2: move  PA1: play", Font_7x10,
                       HOME_COLOR_CYAN, HOME_COLOR_BLACK);
}

void AppUI_DrawUploadWait(void)
{
    draw_screen_header("UPLOAD");
    ST7735_WriteString(8, 48, "Open website", Font_7x10, HOME_COLOR_WHITE, HOME_COLOR_BLACK);
    ST7735_WriteString(8, 60, "Send to STM32", Font_7x10, HOME_COLOR_CYAN, HOME_COLOR_BLACK);
    ST7735_WriteString(8, 76, "Waiting...", Font_7x10, HOME_COLOR_MAGENTA, HOME_COLOR_BLACK);
    ST7735_WriteString(8, 110, "PA1: cancel", Font_7x10, HOME_COLOR_CYAN, HOME_COLOR_BLACK);
}

void AppUI_DrawUploading(uint16_t note_count)
{
    char line[24];

    draw_screen_header("UPLOAD");
    ST7735_WriteString(20, 58, "UPLOADING", Font_11x18, HOME_COLOR_MAGENTA, HOME_COLOR_BLACK);
    ST7735_WriteString(8, 82, "Receiving song", Font_7x10, HOME_COLOR_CYAN, HOME_COLOR_BLACK);
    snprintf(line, sizeof(line), "Notes: %u", (unsigned)note_count);
    ST7735_WriteString(8, 96, line, Font_7x10, HOME_COLOR_WHITE, HOME_COLOR_BLACK);
}

void AppUI_DrawUploadDone(uint16_t note_count)
{
    char line[24];

    draw_screen_header("UPLOAD");
    ST7735_WriteString(36, 58, "DONE!", Font_11x18, HOME_COLOR_CYAN, HOME_COLOR_BLACK);
    snprintf(line, sizeof(line), "%u notes loaded", (unsigned)note_count);
    ST7735_WriteString(8, 82, line, Font_7x10, HOME_COLOR_WHITE, HOME_COLOR_BLACK);
    ST7735_WriteString(8, 110, "Back to menu...", Font_7x10, HOME_COLOR_MAGENTA, HOME_COLOR_BLACK);
}
