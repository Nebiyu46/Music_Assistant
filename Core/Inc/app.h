#ifndef APP_H
#define APP_H

#include "song.h"

typedef enum {
    APP_STATE_HOME,
    APP_STATE_MENU,
    APP_STATE_SONG_LIST,
    APP_STATE_UPLOAD_WAIT,
    APP_STATE_UPLOADING,
    APP_STATE_UPLOAD_DONE,
    APP_STATE_PLAYING
} AppState_t;

void App_Init(void);
void App_Run(void);

const Song_t *App_GetActiveSong(void);
AppState_t    App_GetState(void);

#endif /* APP_H */
