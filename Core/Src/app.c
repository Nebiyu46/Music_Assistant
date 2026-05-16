#include "app.h"
#include "app_ui.h"
#include "ST7735.h"
#include "audio_capture.h"
#include "audio_dsp.h"
#include "demo_song.h"
#include "gameplay.h"
#include "GFX_FUNCTIONS.h"
#include "piano_ui.h"
#include "song_storage.h"
#include "usb_upload.h"
#include "usbd_cdc_if.h"

#include <stdio.h>
#include <string.h>

#define MENU_ITEM_COUNT 2

static AppState_t app_state = APP_STATE_HOME;
static const Song_t *active_song;
static int8_t menu_selection;
static int8_t song_list_selection;
static uint32_t upload_done_tick;
static uint8_t  capture_armed;

static const char *song_catalog_titles[] = {"Happy Birthday"};
#define SONG_CATALOG_COUNT 1

static const Song_t *song_for_gameplay(void)
{
    if (SongStorage_UseUploaded()) {
        return SongStorage_GetRamSong();
    }
    return &demo_song;
}

static int song_catalog_count(void)
{
    return 1;
}

static const char *song_catalog_title(int idx)
{
    (void)idx;
    if (SongStorage_UseUploaded()) {
        return SongStorage_GetUploadTitle();
    }
    return song_catalog_titles[0];
}

static void enter_upload_mode(void)
{
    UsbUpload_ResetBuffer();
    UsbUpload_SetAccepting(1);
    app_state = APP_STATE_UPLOAD_WAIT;
    AppUI_DrawUploadWait();
}

static void enter_gameplay(void)
{
    AudioCapture_Stop();
    HAL_Delay(50);
    AudioCapture_ClearReady();
    capture_armed = 0;
    UsbUpload_SetAccepting(0);
    UsbUpload_ResetBuffer();

    active_song = song_for_gameplay();
    fillScreen(BLACK);
    PianoUI_DrawKeysSection(&active_song->sections[0]);
    Gameplay_Start(active_song);
}

static void run_playing_state(void)
{
    int n_peaks = AUDIO_DSP_PEAK_COUNT / 2;
    float32_t peaks_from_fft[AUDIO_DSP_PEAK_COUNT / 2];
    float32_t peaks_from_yin[AUDIO_DSP_PEAK_COUNT / 2];
    uint8_t is_paused;
    static uint32_t capture_wait_start;

    if (active_song == NULL || active_song->note_count == 0U) {
        HAL_Delay(10);
        return;
    }

    if (!capture_armed) {
        AudioCapture_ClearReady();
        AudioCapture_Start();
        capture_wait_start = HAL_GetTick();
        capture_armed = 1;
    }

    is_paused = Gameplay_IsPaused();
    Gameplay_SpawnBricks(active_song, 1, is_paused);
    Gameplay_PauseExpectedBrickIfDue(active_song, 1, is_paused);
    Gameplay_RenderBricks(active_song, 1);

    if (!AudioCapture_IsReady()) {
        if ((HAL_GetTick() - capture_wait_start) > 600U) {
            AudioCapture_Stop();
            capture_armed = 0;
        }
        return;
    }

    capture_armed = 0;
    AudioCapture_FillFloatBuffers();
    AudioCapture_ClearReady();

#if AUDIO_USE_YIN
    AudioDSP_GetYinPeaks(audio_yin_input_buffer, AUDIO_FFT_LENGTH, peaks_from_yin, n_peaks);
#else
    for (int i = 0; i < n_peaks; i++) {
        peaks_from_yin[i] = 0.0f;
    }
#endif

    AudioDSP_ProcessFftPeaks(peaks_from_fft, n_peaks);
    Gameplay_ValidateBricks(active_song, 1,
                            AUDIO_USE_YIN ? peaks_from_yin : NULL,
                            peaks_from_fft, n_peaks);

#if AUDIO_DEBUG_USB
    {
        char msg[128];
        int len;
        char yin_notes[AUDIO_DSP_PEAK_COUNT / 2][5];
        char fft_notes[AUDIO_DSP_PEAK_COUNT / 2][5];
        int midi_dummy;

        len = sprintf(msg, "YIN: %.2f Hz, %.2f Hz, %.2f Hz\r\n",
                      peaks_from_yin[0], peaks_from_yin[1], peaks_from_yin[2]);
        CDC_Transmit_FS((uint8_t *)msg, (uint16_t)len);
        HAL_Delay(5);

        len = sprintf(msg, "FFT: %.2f Hz, %.2f Hz, %.2f Hz\r\n",
                      peaks_from_fft[0], peaks_from_fft[1], peaks_from_fft[2]);
        CDC_Transmit_FS((uint8_t *)msg, (uint16_t)len);
        HAL_Delay(5);

        for (int i = 0; i < n_peaks; i++) {
            AudioDSP_GetNoteName(peaks_from_fft[i], fft_notes[i], &midi_dummy);
            AudioDSP_GetNoteName(peaks_from_yin[i], yin_notes[i], &midi_dummy);
        }

        len = sprintf(msg,
                      "YIN notes: %s, %s, %s | FFT notes: %s, %s, %s | paused=%u\r\n",
                      yin_notes[0], yin_notes[1], yin_notes[2],
                      fft_notes[0], fft_notes[1], fft_notes[2],
                      (unsigned)Gameplay_IsPaused());
        CDC_Transmit_FS((uint8_t *)msg, (uint16_t)len);
        HAL_Delay(5);
    }
#endif
}

void App_Init(void)
{
    app_state = APP_STATE_HOME;
    menu_selection = 0;
    song_list_selection = 0;
    active_song = &demo_song;

    AudioDSP_Init();
    ST7735_Init(1);
    AppUI_InitHome();
}

void App_Run(void)
{
    const char *catalog_titles[SONG_CATALOG_COUNT];
    int catalog_count;

    switch (app_state) {
    case APP_STATE_HOME:
        AppUI_AnimateHome();
        HAL_Delay(50);
        if (AppUI_PollOkPressed()) {
            menu_selection = 0;
            app_state = APP_STATE_MENU;
            AppUI_DrawMenu(menu_selection);
        }
        break;

    case APP_STATE_MENU:
        if (AppUI_PollNavPressed()) {
            menu_selection = (int8_t)((menu_selection + 1) % MENU_ITEM_COUNT);
            AppUI_DrawMenu(menu_selection);
        }
        if (AppUI_PollOkPressed()) {
            if (menu_selection == 0) {
                song_list_selection = 0;
                app_state = APP_STATE_SONG_LIST;
                catalog_titles[0] = song_catalog_title(0);
                AppUI_DrawSongList(song_list_selection, catalog_titles, song_catalog_count());
            } else {
                AudioCapture_Stop();
                enter_upload_mode();
            }
        }
        HAL_Delay(30);
        break;

    case APP_STATE_SONG_LIST:
        catalog_count = song_catalog_count();
        catalog_titles[0] = song_catalog_title(0);

        if (AppUI_PollNavPressed()) {
            song_list_selection = (int8_t)((song_list_selection + 1) % catalog_count);
            AppUI_DrawSongList(song_list_selection, catalog_titles, catalog_count);
        }
        if (AppUI_PollOkPressed()) {
            active_song = song_for_gameplay();
            enter_gameplay();
            app_state = APP_STATE_PLAYING;
        }
        HAL_Delay(30);
        break;

    case APP_STATE_UPLOAD_WAIT:
        if (UsbUpload_IsRxActive()) {
            app_state = APP_STATE_UPLOADING;
            AppUI_DrawUploading(0);
        }
        if (AppUI_PollOkPressed()) {
            AudioCapture_Stop();
            UsbUpload_SetAccepting(0);
            UsbUpload_ResetBuffer();
            menu_selection = 1;
            app_state = APP_STATE_MENU;
            AppUI_DrawMenu(menu_selection);
        }
        HAL_Delay(20);
        break;

    case APP_STATE_UPLOADING:
        if (UsbUpload_NeedsParsing()) {
            int parsed_count;

            UsbUpload_ClearParsingFlag();
            SongStorage_ParseFromBuffer();
            parsed_count = SongStorage_GetParsedCount();

            if (parsed_count > 0) {
                SongStorage_FinalizeUploaded();
                SongStorage_SetUseUploaded(1);
                active_song = SongStorage_GetRamSong();
                CDC_Transmit_FS((uint8_t *)"OK\r\n", 4);
                upload_done_tick = HAL_GetTick();
                app_state = APP_STATE_UPLOAD_DONE;
                AppUI_DrawUploadDone((uint16_t)parsed_count);
            } else {
                CDC_Transmit_FS((uint8_t *)"ERR\r\n", 5);
                app_state = APP_STATE_UPLOAD_WAIT;
                AppUI_DrawUploadWait();
            }

            UsbUpload_SetAccepting(0);
            UsbUpload_ResetBuffer();
        }
        HAL_Delay(20);
        break;

    case APP_STATE_UPLOAD_DONE:
        if ((HAL_GetTick() - upload_done_tick) > 2500U || AppUI_PollOkPressed()) {
            const char *titles[1];

            menu_selection = 0;
            song_list_selection = 0;
            app_state = APP_STATE_SONG_LIST;
            titles[0] = song_catalog_title(0);
            AppUI_DrawSongList(song_list_selection, titles, song_catalog_count());
        }
        HAL_Delay(30);
        break;

    case APP_STATE_PLAYING:
        run_playing_state();
        break;

    default:
        break;
    }
}

const Song_t *App_GetActiveSong(void)
{
    return active_song;
}

AppState_t App_GetState(void)
{
    return app_state;
}
