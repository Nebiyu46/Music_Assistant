#ifndef SONG_STORAGE_H
#define SONG_STORAGE_H

#include "song.h"

extern SongNote_t song_notes_ram[MAX_SONG_NOTES];

int  SongStorage_GetParsedCount(void);
void SongStorage_ParseFromBuffer(void);
void SongStorage_FinalizeUploaded(void);
const Song_t *SongStorage_GetRamSong(void);
void SongStorage_SetUseUploaded(uint8_t use);
uint8_t SongStorage_UseUploaded(void);
const char *SongStorage_GetUploadTitle(void);

#endif /* SONG_STORAGE_H */
