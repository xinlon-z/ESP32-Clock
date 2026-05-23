#pragma once

#include <stddef.h>
#include <stdint.h>

struct MusicState {
    bool active  = false;
    bool playing = false;

    char title[96]  = "Make a Shadow";
    char artist[96] = "Meg Myers";
    char album[96]  = "Sorry";
    char genre[48]  = "";

    int volume_percent = 64;

    uint32_t progress_start_frame   = 1174943435u;
    uint32_t progress_current_frame = 1181140317u;
    uint32_t progress_end_frame     = 1182711473u;
    uint32_t last_progress_ms       = 0;
};

uint8_t musicProgressPercent(const MusicState& state);
uint32_t musicElapsedSeconds(const MusicState& state);
uint32_t musicDurationSeconds(const MusicState& state);
void musicStateApplyField(MusicState& state, const char* field, const char* payload, size_t payload_len);
