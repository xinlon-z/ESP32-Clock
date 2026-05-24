#pragma once

#include "../core/event/app_events.h"

#include <stdint.h>

enum class SwipeDirection : uint8_t {
    None,
    Left,
    Right,
};

struct TouchPoint {
    int16_t x;
    int16_t y;
};

class SwipeGestureDetector {
public:
    void press(TouchPoint point);
    SwipeDirection release(TouchPoint point);
    void reset();

private:
    bool pressed_ = false;
    TouchPoint start_{0, 0};
};

SwipeDirection detectSwipe(TouchPoint start, TouchPoint end);
ScreenId nextScreenForSwipe(ScreenId current, SwipeDirection direction);
void publishFeatureAction(ScreenId screen, uint8_t action_id);
