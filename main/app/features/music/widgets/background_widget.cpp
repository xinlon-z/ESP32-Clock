#include "background_widget.h"

namespace {
constexpr int kScreenW = 640;
constexpr int kScreenH = 172;
} // namespace

void BackgroundWidget::create(lv_obj_t* parent)
{
    image_obj_ = lv_img_create(parent);
    lv_obj_set_size(image_obj_, kScreenW, kScreenH);
    lv_obj_set_pos(image_obj_, 0, 0);
    lv_obj_set_style_radius(image_obj_, 0, 0);
    lv_obj_set_style_clip_corner(image_obj_, false, 0);
    lv_obj_add_flag(image_obj_, LV_OBJ_FLAG_HIDDEN);
}

void BackgroundWidget::renderCover(const BorrowedCover& cover)
{
    if (!image_obj_ || cover.cover_id == 0 || !cover.image || !cover.pixels) {
        renderPlaceholder();
        return;
    }
    if (cover.cover_id == cover_id_ && image_.pixels) {
        return;
    }

    MusicBackgroundImage next{};
    if (!musicBuildBackgroundImage(cover, kScreenW, kScreenH, &next)) {
        renderPlaceholder();
        return;
    }

    lv_img_cache_invalidate_src(&image_.image);
    musicReleaseBackgroundImage(&stale_image_);
    stale_image_ = image_;
    image_ = next;
    cover_id_ = cover.cover_id;

    lv_img_set_src(image_obj_, &image_.image);
    lv_obj_clear_flag(image_obj_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(image_obj_);
    lv_obj_invalidate(image_obj_);
}

void BackgroundWidget::renderPlaceholder()
{
    cover_id_ = 0;
    if (image_obj_) {
        lv_obj_add_flag(image_obj_, LV_OBJ_FLAG_HIDDEN);
    }
}

void BackgroundWidget::clear()
{
    image_obj_ = nullptr;
    cover_id_ = 0;
    musicReleaseBackgroundImage(&image_);
    musicReleaseBackgroundImage(&stale_image_);
}
