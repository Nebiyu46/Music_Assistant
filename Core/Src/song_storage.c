#include "song_storage.h"
#include "usb_upload.h"
#include "sd_storage.h"
#include "usbd_cdc_if.h"
#include "main.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SongNote_t song_notes_ram[MAX_SONG_NOTES];

static int parsed_note_count;
static Song_t ram_song;
static SongSection_t ram_sections[16];
static char uploaded_display_title[24] = "SD Song";

static void SongStorage_FinalizeParsedSong(void);

int SongStorage_GetParsedCount(void)
{
    return parsed_note_count;
}

static void parse_csv_buffer(char *buffer)
{
    char *rest = buffer;
    char *line;
    parsed_note_count = 0;

    while ((line = strtok_r(rest, "\r\n", &rest)) != NULL) {
        if (strstr(line, "START") || strstr(line, "END")) {
            continue;
        }

        uint32_t start, dur;
        int note;

        if (sscanf(line, "%lu,%d,%lu", &start, &note, &dur) == 3 ||
            sscanf(line, "%lu %d %lu", &start, &note, &dur) == 3) 
        {
            if (parsed_note_count < MAX_SONG_NOTES) {
                song_notes_ram[parsed_note_count].start_ms = (uint16_t)start;
                song_notes_ram[parsed_note_count].midi_note = (uint8_t)note;
                song_notes_ram[parsed_note_count].duration_ms = (uint16_t)dur;
                song_notes_ram[parsed_note_count].key_idx = 0;
                parsed_note_count++;
            }
        }
    }
}

void SongStorage_ParseFromBuffer(void)
{
    parse_csv_buffer(UsbUpload_GetBuffer());
    SongStorage_FinalizeParsedSong();
}

uint8_t SongStorage_LoadFromSD(const char *filename)
{
    static char sd_buffer[4096];

    if (!SD_LoadSong(filename, sd_buffer, sizeof(sd_buffer))) {
        return 0;
    }

    char debug_msg[128];
    int len = snprintf(debug_msg, sizeof(debug_msg), "\r\n--- LOADING: %s ---\r\nSIZE: %u bytes\r\nRAW DATA:\r\n", filename, (unsigned int)strlen(sd_buffer));
    CDC_Transmit_FS((uint8_t *)debug_msg, len);
    HAL_Delay(15);

    CDC_Transmit_FS((uint8_t *)sd_buffer, strlen(sd_buffer) > 100 ? 100 : strlen(sd_buffer));
    HAL_Delay(15);

    parse_csv_buffer(sd_buffer);

    len = snprintf(debug_msg, sizeof(debug_msg), "\r\nPARSED NOTES: %d\r\n", parsed_note_count);
    CDC_Transmit_FS((uint8_t *)debug_msg, len);
    HAL_Delay(15);

    if (parsed_note_count <= 0) {
        return 0;
    }

    SongStorage_FinalizeParsedSong();

    strncpy(uploaded_display_title, filename, sizeof(uploaded_display_title) - 1);
    uploaded_display_title[sizeof(uploaded_display_title) - 1] = '\0';

    char *dot = strrchr(uploaded_display_title, '.');
    if (dot) {
        *dot = '\0';
    }

    return 1;
}

static void SongStorage_FinalizeParsedSong(void)
{
    if (parsed_note_count <= 0) {
        return;
    }

    for (int i = 0; i < parsed_note_count - 1; i++) {
        for (int j = i + 1; j < parsed_note_count; j++) {
            if (song_notes_ram[j].start_ms < song_notes_ram[i].start_ms) {
                SongNote_t tmp = song_notes_ram[i];
                song_notes_ram[i] = song_notes_ram[j];
                song_notes_ram[j] = tmp;
            }
        }
    }

    for (int i = 0; i < parsed_note_count; i++) {
        if (song_notes_ram[i].duration_ms < 80U) {
            song_notes_ram[i].duration_ms = 80U;
        }
        if (song_notes_ram[i].duration_ms > 8000U) {
            song_notes_ram[i].duration_ms = 8000U;
        }
    }

    static const char *octave_labels[8][8] = {
        {"C1", "D1", "E1", "F1", "G1", "A1", "B1", "C2"},
        {"C2", "D2", "E2", "F2", "G2", "A2", "B2", "C3"},
        {"C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4"},
        {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"},
        {"C5", "D5", "E5", "F5", "G5", "A5", "B5", "C6"},
        {"C6", "D6", "E6", "F6", "G6", "A6", "B6", "C7"},
        {"C7", "D7", "E7", "F7", "G7", "A7", "B7", "C8"},
        {"C8", "D8", "E8", "F8", "G8", "A8", "B8", "C9"},
    };

    static const int8_t midi_to_key[12] = {
        0, 0, 1, 1, 2, 3,
        3, 4, 4, 5, 5, 6,
    };

    int section_count = 0;
    int current_octave = ((int)song_notes_ram[0].midi_note / 12) - 1;

    if (current_octave < 0) current_octave = 0;
    if (current_octave > 7) current_octave = 7;

    uint8_t current_base = (uint8_t)((current_octave + 1) * 12);
    int label_idx = current_octave - 1;

    if (label_idx < 0) label_idx = 0;
    if (label_idx > 7) label_idx = 7;

    ram_sections[0].first_note_idx = 0;
    ram_sections[0].base_midi = current_base;

    for (int k = 0; k < SONG_KEYS_PER_SECTION; k++) {
        ram_sections[0].labels[k] = octave_labels[label_idx][k];
    }

    section_count = 1;

    for (int i = 0; i < parsed_note_count; i++) {
        int note_octave = ((int)song_notes_ram[i].midi_note / 12) - 1;
        if (note_octave < 0) note_octave = 0;
        if (note_octave > 7) note_octave = 7;

        if (note_octave != current_octave && section_count < 16) {
            current_octave = note_octave;
            current_base = (uint8_t)((current_octave + 1) * 12);
            label_idx = current_octave - 1;

            if (label_idx < 0) label_idx = 0;
            if (label_idx > 7) label_idx = 7;

            ram_sections[section_count].first_note_idx = (uint16_t)i;
            ram_sections[section_count].base_midi = current_base;

            for (int k = 0; k < SONG_KEYS_PER_SECTION; k++) {
                ram_sections[section_count].labels[k] = octave_labels[label_idx][k];
            }
            section_count++;
        }

        int note_in_octave = (int)song_notes_ram[i].midi_note % 12;
        song_notes_ram[i].key_idx = (uint8_t)midi_to_key[note_in_octave];
    }

    ram_song.notes = song_notes_ram;
    ram_song.note_count = (uint16_t)parsed_note_count;
    ram_song.sections = ram_sections;
    ram_song.section_count = (uint8_t)section_count;
}

const Song_t *SongStorage_GetRamSong(void)
{
    return &ram_song;
}

const char *SongStorage_GetUploadTitle(void)
{
    return uploaded_display_title;
}