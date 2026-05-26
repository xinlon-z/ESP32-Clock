#pragma once

#include "app/services/cover_service.h"
#include "lvgl.h"

#include <stdint.h>

struct MusicBackgroundImage {
    lv_img_dsc_t image{};
    lv_color_t* pixels = nullptr;
};

bool musicBuildBackgroundImage(const BorrowedCover& cover,
                               uint16_t width,
                               uint16_t height,
                               MusicBackgroundImage* background);
void musicReleaseBackgroundImage(MusicBackgroundImage* background);
