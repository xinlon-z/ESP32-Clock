#pragma once

#include "clock_face_screen.h"
#include "lvgl.h"
#include "music_player_screen.h"
#include "screen_nav.h"

class ScreenManager {
public:
    static ScreenManager& instance();

    void create();
    void destroy();
    void attachGestureHandler(lv_obj_t* root);

private:
    static void onGestureEvent(lv_event_t* event);

    void detachGestureHandler();
    void switchTo(ScreenId target);
    void handleSwipe(SwipeDirection swipe);

    ScreenId current_ = ScreenId::Clock;
    ClockFaceScreen clock_;
    MusicPlayerScreen music_;
    SwipeGestureDetector swipe_detector_;
    lv_obj_t* gesture_root_ = nullptr;
};
