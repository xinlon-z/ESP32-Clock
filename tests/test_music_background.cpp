#include "app/features/music/util/music_background.cpp"

#include <stdio.h>
#include <vector>

static uint8_t redOf(lv_color_t color)
{
    return static_cast<uint8_t>((lv_color_to32(color) >> 16) & 0xff);
}

static uint8_t blueOf(lv_color_t color)
{
    return static_cast<uint8_t>(lv_color_to32(color) & 0xff);
}

int main()
{
    constexpr uint16_t cover_w = 16;
    constexpr uint16_t cover_h = 16;
    constexpr uint16_t out_w = 64;
    constexpr uint16_t out_h = 16;

    std::vector<lv_color_t> cover(cover_w * cover_h);
    std::vector<lv_color_t> output(out_w * out_h);
    std::vector<lv_color_t> scratch(out_w * out_h);

    for (uint16_t y = 0; y < cover_h; ++y) {
        for (uint16_t x = 0; x < cover_w; ++x) {
            cover[y * cover_w + x] = x < cover_w / 2
                                         ? lv_color_make(240, 20, 30)
                                         : lv_color_make(20, 40, 240);
        }
    }

    if (!musicGenerateBlurredBackground(cover.data(), cover_w, cover_h,
                                        output.data(), out_w, out_h, scratch.data())) {
        printf("background generation failed\n");
        return 1;
    }

    const lv_color_t left = output[out_h / 2 * out_w + 2];
    const lv_color_t center = output[out_h / 2 * out_w + out_w / 2];
    const lv_color_t right = output[out_h / 2 * out_w + out_w - 3];

    if (redOf(left) <= blueOf(left)) {
        printf("expected left side to keep red cover influence\n");
        return 1;
    }
    if (blueOf(right) <= redOf(right)) {
        printf("expected right side to keep blue cover influence\n");
        return 1;
    }
    if (redOf(center) == 0 || blueOf(center) == 0) {
        printf("expected blurred center to contain mixed color\n");
        return 1;
    }

    return 0;
}
