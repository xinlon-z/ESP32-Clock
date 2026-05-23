#include "../main/music_player_icon_geometry.h"

#include <stdio.h>

static int expectOffset(const char* name, bool playing, int expected_x, int expected_y)
{
    const MusicIconOffset actual = musicPlayPauseIconOffset(playing);
    if (actual.x != expected_x || actual.y != expected_y) {
        printf("%s expected (%d,%d) got (%d,%d)\n",
               name, expected_x, expected_y, actual.x, actual.y);
        return 1;
    }
    return 0;
}

int main()
{
    int failures = 0;
    failures += expectOffset("pause icon", true, 0, 0);
    failures += expectOffset("play icon", false, 2, 0);
    return failures == 0 ? 0 : 1;
}
