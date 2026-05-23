#include "../main/screen_nav.cpp"

#include <stdio.h>

static int expectSwipe(const char* name, TouchPoint start, TouchPoint end, SwipeDirection expected)
{
    const SwipeDirection actual = detectSwipe(start, end);
    if (actual != expected) {
        printf("%s expected %d got %d\n",
               name, static_cast<int>(expected), static_cast<int>(actual));
        return 1;
    }
    return 0;
}

static int expectNext(const char* name, ScreenId current, SwipeDirection direction, ScreenId expected)
{
    const ScreenId actual = nextScreenForSwipe(current, direction);
    if (actual != expected) {
        printf("%s expected %d got %d\n",
               name, static_cast<int>(expected), static_cast<int>(actual));
        return 1;
    }
    return 0;
}

int main()
{
    int failures = 0;
    failures += expectSwipe("left swipe", {200, 100}, {40, 110}, SwipeDirection::Left);
    failures += expectSwipe("right swipe", {40, 100}, {200, 110}, SwipeDirection::Right);
    failures += expectSwipe("short drag", {40, 100}, {120, 110}, SwipeDirection::None);
    failures += expectSwipe("vertical drag", {40, 100}, {200, 180}, SwipeDirection::None);

    failures += expectNext("clock left", ScreenId::Clock, SwipeDirection::Left, ScreenId::Music);
    failures += expectNext("music right", ScreenId::Music, SwipeDirection::Right, ScreenId::Clock);
    failures += expectNext("clock right ignored", ScreenId::Clock, SwipeDirection::Right, ScreenId::Clock);
    failures += expectNext("music left ignored", ScreenId::Music, SwipeDirection::Left, ScreenId::Music);
    failures += expectNext("clock none ignored", ScreenId::Clock, SwipeDirection::None, ScreenId::Clock);
    failures += expectNext("music none ignored", ScreenId::Music, SwipeDirection::None, ScreenId::Music);

    SwipeGestureDetector detector;
    detector.press({200, 100});
    if (detector.release({40, 110}) != SwipeDirection::Left) {
        printf("detector left swipe failed\n");
        failures++;
    }
    if (detector.release({40, 110}) != SwipeDirection::None) {
        printf("detector released state failed\n");
        failures++;
    }
    detector.press({40, 100});
    detector.reset();
    if (detector.release({200, 110}) != SwipeDirection::None) {
        printf("detector reset failed\n");
        failures++;
    }

    return failures == 0 ? 0 : 1;
}
