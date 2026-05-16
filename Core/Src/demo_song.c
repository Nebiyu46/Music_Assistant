#include "demo_song.h"

static const SongNote_t demo_song_notes[] = {
    {     0,  250, 67, 4 },
    {   250,  250, 67, 4 },
    {   500,  500, 69, 5 },
    {  1000,  500, 67, 4 },
    {  1500,  500, 72, 7 },
    {  2000, 1000, 71, 6 },
    {  3000,  250, 67, 4 },
    {  3250,  250, 67, 4 },
    {  3500,  500, 69, 5 },
    {  4000,  500, 67, 4 },
    {  4500,  500, 74, 1 },
    {  5000, 1000, 72, 7 },
    {  6000,  250, 67, 4 },
    {  6250,  250, 67, 4 },
    {  6500,  500, 79, 4 },
    {  7000,  500, 76, 2 },
    {  7500,  500, 72, 7 },
    {  8000,  500, 71, 6 },
    {  8500, 1000, 69, 5 },
    {  9500,  250, 77, 3 },
    {  9750,  250, 77, 3 },
    { 10000,  500, 76, 2 },
    { 10500,  500, 72, 7 },
    { 11000,  500, 74, 1 },
    { 11500, 1500, 72, 7 },
};

static const SongSection_t demo_song_sections[] = {
    { 0,  60, { "C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5" } },
    { 16, 72, { "C5", "D5", "E5", "F5", "G5", "A5", "B5", "C6" } },
};

const Song_t demo_song = {
    demo_song_notes,
    (uint16_t)(sizeof(demo_song_notes) / sizeof(demo_song_notes[0])),
    demo_song_sections,
    (uint8_t)(sizeof(demo_song_sections) / sizeof(demo_song_sections[0])),
};
