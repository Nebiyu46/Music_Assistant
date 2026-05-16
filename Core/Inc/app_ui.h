#ifndef APP_UI_H
#define APP_UI_H

#include <stdint.h>

void AppUI_InitHome(void);
void AppUI_AnimateHome(void);
void AppUI_DrawMenu(int8_t selection);
void AppUI_DrawSongList(int8_t selection, const char *const *titles, int count);
void AppUI_DrawUploadWait(void);
void AppUI_DrawUploading(uint16_t note_count);
void AppUI_DrawUploadDone(uint16_t note_count);

uint8_t AppUI_PollOkPressed(void);
uint8_t AppUI_PollNavPressed(void);

#endif /* APP_UI_H */
