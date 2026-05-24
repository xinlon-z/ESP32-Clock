#include "gesture_manager.h"

#include "../core/event/event_bus.h"

namespace {
constexpr int kMinSwipeX = 120;
constexpr int kMaxSwipeY = 54;

int absInt(int v)
{
    return v < 0 ? -v : v;
}
} // namespace

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
    start_ = {0, 0};
}

SwipeDirection detectSwipe(TouchPoint start, TouchPoint end)
{
    const int dx = end.x - start.x;
    const int dy = end.y - start.y;
    if (absInt(dx) < kMinSwipeX || absInt(dy) > kMaxSwipeY) {
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

void publishFeatureAction(ScreenId screen, uint8_t action_id)
{
    AppEvent event{};
    event.type = AppEventType::FeatureAction;
    event.payload.feature_action.screen_id = static_cast<uint8_t>(screen);
    event.payload.feature_action.action_id = action_id;
    EventBus::get().publish(event);
}
