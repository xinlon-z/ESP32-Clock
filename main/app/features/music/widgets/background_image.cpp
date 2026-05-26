#include "background_image.h"

#include "esp_heap_caps.h"
#include "music_background.h"

namespace {
lv_color_t* allocPixels(size_t count)
{
    return static_cast<lv_color_t*>(heap_caps_malloc(count * sizeof(lv_color_t),
                                                     MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
}
} // namespace

bool musicBuildBackgroundImage(const BorrowedCover& cover,
                               uint16_t width,
                               uint16_t height,
                               MusicBackgroundImage* background)
{
    if (!background || !cover.pixels || !cover.image || width == 0 || height == 0 ||
        cover.image->header.w == 0 || cover.image->header.h == 0) {
        return false;
    }

    MusicBackgroundImage next{};
    next.pixels = allocPixels(static_cast<size_t>(width) * height);
    if (!next.pixels) {
        return false;
    }

    lv_color_t* scratch = allocPixels(static_cast<size_t>(width) * height);
    if (!scratch) {
        heap_caps_free(next.pixels);
        return false;
    }

    const bool ok = musicGenerateBlurredBackground(cover.pixels,
                                                  static_cast<uint16_t>(cover.image->header.w),
                                                  static_cast<uint16_t>(cover.image->header.h),
                                                  next.pixels,
                                                  width,
                                                  height,
                                                  scratch);
    heap_caps_free(scratch);
    if (!ok) {
        heap_caps_free(next.pixels);
        return false;
    }

    next.image.header.always_zero = 0;
    next.image.header.w = width;
    next.image.header.h = height;
    next.image.header.cf = LV_IMG_CF_TRUE_COLOR;
    next.image.data = reinterpret_cast<const uint8_t*>(next.pixels);
    next.image.data_size = static_cast<uint32_t>(width) * height * sizeof(lv_color_t);

    musicReleaseBackgroundImage(background);
    *background = next;
    return true;
}

void musicReleaseBackgroundImage(MusicBackgroundImage* background)
{
    if (!background) {
        return;
    }
    if (background->pixels) {
        heap_caps_free(background->pixels);
    }
    *background = MusicBackgroundImage{};
}
