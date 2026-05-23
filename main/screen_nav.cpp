#include "screen_nav.h"

#include <cstdlib>

namespace {
constexpr int kMinSwipeX = 120;
constexpr int kMaxSwipeY = 54;
}

void SwipeGestureDetector::press(TouchPoint point)
{
    pressed_ = true;
    start_ = point;
}

SwipeDirection SwipeGestureDetector::release(TouchPoint point)
{
    if (!pressed_) {
        return SwipeDirection::None;
    }

    pressed_ = false;
    return detectSwipe(start_, point);
}

void SwipeGestureDetector::reset()
{
    pressed_ = false;
    start_ = TouchPoint{0, 0};
}

SwipeDirection detectSwipe(TouchPoint start, TouchPoint end)
{
    const int dx = static_cast<int>(end.x) - static_cast<int>(start.x);
    const int dy = static_cast<int>(end.y) - static_cast<int>(start.y);

    if (std::abs(dx) < kMinSwipeX || std::abs(dy) > kMaxSwipeY) {
        return SwipeDirection::None;
    }

    return dx < 0 ? SwipeDirection::Left : SwipeDirection::Right;
}

ScreenId nextScreenForSwipe(ScreenId current, SwipeDirection direction)
{
    if (current == ScreenId::Clock && direction == SwipeDirection::Left) {
        return ScreenId::Music;
    }

    if (current == ScreenId::Music && direction == SwipeDirection::Right) {
        return ScreenId::Clock;
    }

    return current;
}
