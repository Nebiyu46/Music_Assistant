#ifndef SD_STORAGE_H
#define SD_STORAGE_H

#include <stdint.h>

#define SD_MAX_SONGS     10
#define SD_FILENAME_LEN  24

void     SD_Init(void);
uint8_t  SD_SaveSong(const char *filename, const char *csv_data, uint32_t len);
uint8_t  SD_LoadSong(const char *filename, char *buf, uint32_t buf_size);
uint8_t  SD_ScanSongs(void);
int      SD_GetSongCount(void);
const char *SD_GetSongTitle(int idx);
const char *SD_GetSongFilename(int idx);

#endif