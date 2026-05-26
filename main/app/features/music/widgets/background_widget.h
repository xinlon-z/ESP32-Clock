#pragma once

#include "background_image.h"
#include "lvgl.h"

#include <stdint.h>

class BackgroundWidget {
public:
    void create(lv_obj_t* parent);
    void renderCover(const BorrowedCover& cover);
    void renderPlaceholder();
    void clear();

private:
    lv_obj_t* image_obj_ = nullptr;
    uint32_t cover_id_ = 0;
    MusicBackgroundImage image_{};
    MusicBackgroundImage stale_image_{};
};
