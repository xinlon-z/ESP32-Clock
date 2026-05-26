#pragma once

#include <stdint.h>

struct lv_obj_t {};
typedef unsigned short lv_color_t;

struct lv_img_header_t {
    unsigned int always_zero = 0;
    unsigned int w = 0;
    unsigned int h = 0;
    unsigned int cf = 0;
};

struct lv_img_dsc_t {
    lv_img_header_t header{};
    const unsigned char* data = nullptr;
    unsigned int data_size = 0;
};

#define LV_IMG_CF_TRUE_COLOR 1

static inline lv_color_t lv_color_make(unsigned char r, unsigned char g, unsigned char b)
{
    return static_cast<lv_color_t>(((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3));
}

static inline unsigned int lv_color_to32(lv_color_t color)
{
    const unsigned int r = ((color >> 11) & 0x1f) << 3;
    const unsigned int g = ((color >> 5) & 0x3f) << 2;
    const unsigned int b = (color & 0x1f) << 3;
    return (r << 16) | (g << 8) | b;
}

static inline uint32_t lv_tick_elaps(uint32_t) { return 0; }
