#include "sd_storage.h"
#include "fatfs.h"
#include <string.h>
#include <stdio.h>
#include "usbd_cdc_if.h"

static FATFS fs;
static uint8_t mounted = 0;

static char song_titles[SD_MAX_SONGS][SD_FILENAME_LEN];
static char song_filenames[SD_MAX_SONGS][SD_FILENAME_LEN];
static int  song_count = 0;

void SD_Init(void)
{
    FRESULT res = f_mount(&fs, "", 1);
    if (res == FR_OK) {
        mounted = 1;
        CDC_Transmit_FS((uint8_t *)"SD Mount OK\r\n", 13);
    }
    else{
        mounted = 0;
        CDC_Transmit_FS((uint8_t *)"SD Mount FAIL\r\n", 15);
    }
}

uint8_t SD_SaveSong(const char *filename, const char *csv_data, uint32_t len)
{
    FIL fil;
    UINT bw;
    FRESULT res;

    if (!mounted) return 0;

    res = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) return 0;

    f_write(&fil, csv_data, len, &bw);
    f_close(&fil);
    return (bw == len) ? 1 : 0;
}

uint8_t SD_LoadSong(const char *filename, char *buf, uint32_t buf_size)
{
    FIL fil;
    UINT br;
    FRESULT res;

    if (!mounted) return 0;

    res = f_open(&fil, filename, FA_READ);
    if (res != FR_OK) return 0;

    f_read(&fil, buf, buf_size - 1, &br);
    buf[br] = '\0';
    f_close(&fil);
    return 1;
}

uint8_t SD_ScanSongs(void)
{
    DIR dir;
    FILINFO fno;
    FRESULT res;

    if (!mounted) return 0;

    song_count = 0;
    res = f_opendir(&dir, "/"); 
    if (res != FR_OK) return 0;

    while (song_count < SD_MAX_SONGS) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) break;
        if (fno.fattrib & AM_DIR) continue;

        if (strstr(fno.fname, ".TXT") != NULL || strstr(fno.fname, ".txt") != NULL) {
            strncpy(song_filenames[song_count], fno.fname, SD_FILENAME_LEN - 1);
            song_filenames[song_count][SD_FILENAME_LEN - 1] = '\0';

            strncpy(song_titles[song_count], fno.fname, SD_FILENAME_LEN - 1);
            song_titles[song_count][SD_FILENAME_LEN - 1] = '\0';
            
            char *ext = strrchr(song_titles[song_count], '.');
            if (ext) *ext = '\0';

            song_count++;
        }
    }
    f_closedir(&dir);
    return 1;
}

int SD_GetSongCount(void)
{
    return song_count;
}

const char *SD_GetSongTitle(int idx)
{
    if (idx < 0 || idx >= song_count) return "";
    return song_titles[idx];
}

const char *SD_GetSongFilename(int idx)
{
    if (idx < 0 || idx >= song_count) return "";
    return song_filenames[idx];
}