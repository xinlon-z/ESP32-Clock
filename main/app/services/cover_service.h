#pragma once

#include "../core/event/app_events.h"
#include "lvgl.h"

#include <mutex>
#include <stdint.h>

struct CoverBuffer {
    uint32_t cover_id = 0;
    CoverStatus status = CoverStatus::Idle;
    uint8_t* jpeg_data = nullptr;
    uint32_t jpeg_size = 0;
    lv_img_dsc_t image{};
    lv_color_t* pixels = nullptr;
};

class CoverService {
public:
    static CoverService& get();

    uint32_t acceptJpeg(uint8_t* data, uint32_t size);
    CoverBuffer active();
    void clear();

private:
    CoverService() = default;

    void releaseActive();
    void publishChanged(uint32_t cover_id, CoverStatus status);

    mutable std::mutex mutex_;
    uint32_t next_cover_id_ = 0;
    CoverBuffer active_{};
};
