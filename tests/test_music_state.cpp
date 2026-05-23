#include "../main/music_state.cpp"

#include <stdio.h>
#include <string.h>

int main()
{
    MusicState s;
    s.progress_start_frame   = 1000;
    s.progress_current_frame = 1500;
    s.progress_end_frame     = 2000;
    if (musicProgressPercent(s) != 50) {
        printf("expected 50 got %u\n", musicProgressPercent(s));
        return 1;
    }

    s.progress_current_frame = 3000;
    if (musicProgressPercent(s) != 100) {
        printf("expected clamp to 100 got %u\n", musicProgressPercent(s));
        return 1;
    }

    s.progress_end_frame = 1000;
    if (musicProgressPercent(s) != 0) {
        printf("expected invalid range 0 got %u\n", musicProgressPercent(s));
        return 1;
    }

    s.progress_start_frame = 0xfffffff0u;
    s.progress_current_frame = 0x00000010u;
    s.progress_end_frame = 0x00000030u;
    if (musicProgressPercent(s) != 50) {
        printf("expected wrapped progress 50 got %u\n", musicProgressPercent(s));
        return 1;
    }

    s.progress_start_frame = 1000;
    s.progress_current_frame = 1000 + 44100 * 42;
    s.progress_end_frame = 1000 + 44100 * 120;
    if (musicElapsedSeconds(s) != 42 || musicDurationSeconds(s) != 120) {
        printf("expected 42/120 seconds got %lu/%lu\n",
               static_cast<unsigned long>(musicElapsedSeconds(s)),
               static_cast<unsigned long>(musicDurationSeconds(s)));
        return 1;
    }

    musicStateApplyField(s, "title", "Live Track", 10);
    if (strcmp(s.title, "Live Track") != 0) {
        printf("expected title update got %s\n", s.title);
        return 1;
    }

    musicStateApplyField(s, "active", "1", 1);
    musicStateApplyField(s, "playing", "0", 1);
    if (!s.active || s.playing) {
        printf("expected active true and playing false\n");
        return 1;
    }

    musicStateApplyField(s, "volume", "-24.0", 5);
    if (s.volume_percent != 50) {
        printf("expected volume 50 got %d\n", s.volume_percent);
        return 1;
    }

    musicStateApplyField(s, "ssnc/prgr", "1174943435/1181140317/1182711473", 32);
    if (s.progress_start_frame != 1174943435u ||
        s.progress_current_frame != 1181140317u ||
        s.progress_end_frame != 1182711473u) {
        printf("expected prgr update\n");
        return 1;
    }

    return 0;
}
