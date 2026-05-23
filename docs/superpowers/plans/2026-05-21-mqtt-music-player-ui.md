# MQTT Music Player UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a Liquid Glass inspired music player screen to the ESP32 display while keeping the existing clock as the default screen and supporting left/right swipe navigation.

**Architecture:** Introduce a small screen navigation layer that owns both screens and switches between them. Keep gesture recognition as a pure C++ helper with unit coverage, keep LVGL rendering inside screen classes, and keep future MQTT ingestion behind a `MusicState` data model so network code does not couple directly to widgets.

**Tech Stack:** ESP-IDF C++, LVGL, existing `Screen` abstraction, existing touch driver, standalone C++ tests for pure logic, `idf.py build` for firmware verification.

---

## File Structure

- Create `main/screen_nav.h`: Pure gesture detection API and `ScreenId` enum.
- Create `main/screen_nav.cpp`: Swipe threshold logic.
- Create `tests/test_screen_nav.cpp`: Standalone test for left/right/no-swipe detection.
- Create `main/music_state.h`: State model for Shairport Sync playback data.
- Create `main/music_player_screen.h`: `MusicPlayerScreen : Screen` declaration.
- Create `main/music_player_screen.cpp`: Static Liquid Glass style LVGL music UI using sample playback data.
- Create `main/screen_manager.h`: Screen manager API and LVGL gesture event callback.
- Create `main/screen_manager.cpp`: Owns clock/music screens and switches on swipe.
- Modify `main/main.cpp`: Initialize `ScreenManager` instead of creating `ClockFaceScreen` directly.
- Modify `main/CMakeLists.txt`: Add new firmware source files.

This plan implements the first firmware pass: UI shell, static music screen, and swipe navigation. MQTT ingestion and cover decoding are intentionally left for the next pass after the UI and navigation are stable on hardware.

---

### Task 1: Swipe Gesture Logic

**Files:**
- Create: `main/screen_nav.h`
- Create: `main/screen_nav.cpp`
- Create: `tests/test_screen_nav.cpp`

- [ ] **Step 1: Add pure navigation types**

Create `main/screen_nav.h`:

```cpp
#pragma once

#include <stdint.h>

enum class ScreenId {
    Clock,
    Music,
};

enum class SwipeDirection {
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
    bool       pressed_ = false;
    TouchPoint start_   = {0, 0};
};

SwipeDirection detectSwipe(TouchPoint start, TouchPoint end);
ScreenId nextScreenForSwipe(ScreenId current, SwipeDirection swipe);
```

- [ ] **Step 2: Write failing tests**

Create `tests/test_screen_nav.cpp`:

```cpp
#include "../main/screen_nav.cpp"

#include <stdio.h>

static int expectSwipe(const char* name, TouchPoint start, TouchPoint end, SwipeDirection expected)
{
    const SwipeDirection actual = detectSwipe(start, end);
    if (actual != expected) {
        printf("%s expected %d got %d\n", name, static_cast<int>(expected), static_cast<int>(actual));
        return 1;
    }
    return 0;
}

static int expectNext(const char* name, ScreenId current, SwipeDirection swipe, ScreenId expected)
{
    const ScreenId actual = nextScreenForSwipe(current, swipe);
    if (actual != expected) {
        printf("%s expected %d got %d\n", name, static_cast<int>(expected), static_cast<int>(actual));
        return 1;
    }
    return 0;
}

int main()
{
    int failures = 0;
    failures += expectSwipe("left swipe",  {520, 80}, {250, 78}, SwipeDirection::Left);
    failures += expectSwipe("right swipe", {120, 80}, {405, 84}, SwipeDirection::Right);
    failures += expectSwipe("short drag",  {320, 80}, {270, 76}, SwipeDirection::None);
    failures += expectSwipe("vertical drag", {320, 30}, {302, 132}, SwipeDirection::None);
    failures += expectNext("clock left", ScreenId::Clock, SwipeDirection::Left, ScreenId::Music);
    failures += expectNext("clock right ignored", ScreenId::Clock, SwipeDirection::Right, ScreenId::Clock);
    failures += expectNext("music right", ScreenId::Music, SwipeDirection::Right, ScreenId::Clock);
    failures += expectNext("music left ignored", ScreenId::Music, SwipeDirection::Left, ScreenId::Music);
    return failures == 0 ? 0 : 1;
}
```

- [ ] **Step 3: Run test and verify it fails**

Run:

```bash
c++ -std=c++17 -I. tests/test_screen_nav.cpp -o /tmp/test_screen_nav && /tmp/test_screen_nav
```

Expected: compile failure because `main/screen_nav.cpp` does not exist yet.

- [ ] **Step 4: Implement swipe logic**

Create `main/screen_nav.cpp`:

```cpp
#include "screen_nav.h"

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
    start_   = point;
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
    start_   = {0, 0};
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

ScreenId nextScreenForSwipe(ScreenId current, SwipeDirection swipe)
{
    if (current == ScreenId::Clock && swipe == SwipeDirection::Left) {
        return ScreenId::Music;
    }
    if (current == ScreenId::Music && swipe == SwipeDirection::Right) {
        return ScreenId::Clock;
    }
    return current;
}
```

- [ ] **Step 5: Verify test passes**

Run:

```bash
c++ -std=c++17 -I. tests/test_screen_nav.cpp -o /tmp/test_screen_nav && /tmp/test_screen_nav
```

Expected: command exits `0`.

---

### Task 2: Music State Model

**Files:**
- Create: `main/music_state.h`

- [ ] **Step 1: Add state model**

Create `main/music_state.h`:

```cpp
#pragma once

#include <stdint.h>

struct MusicState {
    bool active  = false;
    bool playing = false;

    const char* title  = "Make a Shadow";
    const char* artist = "Meg Myers";
    const char* album  = "Sorry";

    int volume_percent = 64;

    uint32_t progress_start_frame   = 1174943435u;
    uint32_t progress_current_frame = 1181140317u;
    uint32_t progress_end_frame     = 1182711473u;
    uint32_t last_progress_ms       = 0;
};

uint8_t musicProgressPercent(const MusicState& state);
```

- [ ] **Step 2: Add progress calculation inline in implementation file**

Create `main/music_state.cpp`:

```cpp
#include "music_state.h"

uint8_t musicProgressPercent(const MusicState& state)
{
    if (state.progress_end_frame <= state.progress_start_frame ||
        state.progress_current_frame <= state.progress_start_frame) {
        return 0;
    }

    const uint32_t elapsed = state.progress_current_frame - state.progress_start_frame;
    const uint32_t total   = state.progress_end_frame - state.progress_start_frame;
    uint32_t pct = (elapsed * 100u) / total;
    if (pct > 100u) pct = 100u;
    return static_cast<uint8_t>(pct);
}
```

- [ ] **Step 3: Add a focused test**

Create `tests/test_music_state.cpp`:

```cpp
#include "../main/music_state.cpp"

#include <stdio.h>

int main()
{
    MusicState s;
    s.progress_start_frame = 1000;
    s.progress_current_frame = 1500;
    s.progress_end_frame = 2000;
    if (musicProgressPercent(s) != 50) {
        printf("expected 50 got %u\n", musicProgressPercent(s));
        return 1;
    }

    s.progress_current_frame = 3000;
    if (musicProgressPercent(s) != 100) {
        printf("expected clamp to 100 got %u\n", musicProgressPercent(s));
        return 1;
    }

    s.progress_end_frame = 1000;
    if (musicProgressPercent(s) != 0) {
        printf("expected invalid range 0 got %u\n", musicProgressPercent(s));
        return 1;
    }

    return 0;
}
```

- [ ] **Step 4: Run test**

Run:

```bash
c++ -std=c++17 -I. tests/test_music_state.cpp -o /tmp/test_music_state && /tmp/test_music_state
```

Expected: command exits `0`.

---

### Task 3: Static Liquid Glass Music Screen

**Files:**
- Create: `main/music_player_screen.h`
- Create: `main/music_player_screen.cpp`

- [ ] **Step 1: Add screen declaration**

Create `main/music_player_screen.h`:

```cpp
#pragma once

#include "lvgl.h"
#include "music_state.h"
#include "screen.h"

class MusicPlayerScreen : public Screen {
public:
    void create() override;
    void destroy() override;

private:
    static void onTimer(lv_timer_t* t);

    void updateUi();
    lv_obj_t* makeLabel(lv_obj_t* parent, const char* text, const lv_font_t* font, uint32_t color);
    lv_obj_t* makePanel(lv_obj_t* parent, int x, int y, int w, int h, uint32_t color, lv_opa_t opa);
    lv_obj_t* makeRoundButton(lv_obj_t* parent, int x, int y, int size, bool primary);
    void makePrevIcon(lv_obj_t* parent, uint32_t color);
    void makePauseIcon(lv_obj_t* parent, uint32_t color);
    void makeNextIcon(lv_obj_t* parent, uint32_t color);

    MusicState state_;
    lv_obj_t*  title_        = nullptr;
    lv_obj_t*  subtitle_     = nullptr;
    lv_obj_t*  progress_     = nullptr;
    lv_obj_t*  elapsed_      = nullptr;
    lv_obj_t*  duration_     = nullptr;
    lv_timer_t* timer_       = nullptr;
};
```

- [ ] **Step 2: Implement screen**

Create `main/music_player_screen.cpp` with the approved LVGL approximation:

```cpp
#include "music_player_screen.h"

#include <stdio.h>

#include "lvgl.h"

namespace {
constexpr uint32_t kBg0       = 0x08080b;
constexpr uint32_t kBg1       = 0x17131a;
constexpr uint32_t kGlass     = 0x2b2b31;
constexpr uint32_t kGlassHi   = 0xffffff;
constexpr uint32_t kText      = 0xf8f8fa;
constexpr uint32_t kMuted     = 0xa8a8ad;
constexpr uint32_t kCoverA    = 0xd85c52;
constexpr uint32_t kCoverB    = 0x24364b;
constexpr uint32_t kCoverC    = 0xe7d4bb;

constexpr int kScreenW = 640;
constexpr int kScreenH = 172;

void clearStyle(lv_obj_t* obj)
{
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

void setBg(lv_obj_t* obj, uint32_t color, lv_opa_t opa)
{
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, opa, 0);
}
} // namespace

lv_obj_t* MusicPlayerScreen::makeLabel(lv_obj_t* parent, const char* text,
                                       const lv_font_t* font, uint32_t color)
{
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
    return label;
}

lv_obj_t* MusicPlayerScreen::makePanel(lv_obj_t* parent, int x, int y, int w, int h,
                                       uint32_t color, lv_opa_t opa)
{
    lv_obj_t* panel = lv_obj_create(parent);
    lv_obj_set_size(panel, w, h);
    lv_obj_set_pos(panel, x, y);
    clearStyle(panel);
    setBg(panel, color, opa);
    lv_obj_set_style_radius(panel, 16, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(kGlassHi), 0);
    lv_obj_set_style_border_opa(panel, LV_OPA_30, 0);
    return panel;
}

lv_obj_t* MusicPlayerScreen::makeRoundButton(lv_obj_t* parent, int x, int y, int size, bool primary)
{
    lv_obj_t* btn = lv_obj_create(parent);
    lv_obj_set_size(btn, size, size);
    lv_obj_set_pos(btn, x, y);
    clearStyle(btn);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    setBg(btn, primary ? 0xf8f8fa : 0xffffff, primary ? LV_OPA_COVER : LV_OPA_20);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_opa(btn, primary ? LV_OPA_60 : LV_OPA_30, 0);
    return btn;
}

void MusicPlayerScreen::makePrevIcon(lv_obj_t* parent, uint32_t color)
{
    lv_obj_t* a = lv_obj_create(parent);
    lv_obj_set_size(a, 8, 16);
    lv_obj_set_pos(a, 9, 9);
    clearStyle(a);
    setBg(a, color, LV_OPA_COVER);
    lv_obj_set_style_transform_angle(a, 450, 0);

    lv_obj_t* b = lv_obj_create(parent);
    lv_obj_set_size(b, 8, 16);
    lv_obj_set_pos(b, 17, 9);
    clearStyle(b);
    setBg(b, color, LV_OPA_COVER);
    lv_obj_set_style_transform_angle(b, 450, 0);
}

void MusicPlayerScreen::makePauseIcon(lv_obj_t* parent, uint32_t color)
{
    lv_obj_t* left = lv_obj_create(parent);
    lv_obj_set_size(left, 6, 18);
    lv_obj_set_pos(left, 15, 14);
    clearStyle(left);
    setBg(left, color, LV_OPA_COVER);
    lv_obj_set_style_radius(left, 2, 0);

    lv_obj_t* right = lv_obj_create(parent);
    lv_obj_set_size(right, 6, 18);
    lv_obj_set_pos(right, 25, 14);
    clearStyle(right);
    setBg(right, color, LV_OPA_COVER);
    lv_obj_set_style_radius(right, 2, 0);
}

void MusicPlayerScreen::makeNextIcon(lv_obj_t* parent, uint32_t color)
{
    lv_obj_t* a = lv_obj_create(parent);
    lv_obj_set_size(a, 8, 16);
    lv_obj_set_pos(a, 10, 9);
    clearStyle(a);
    setBg(a, color, LV_OPA_COVER);
    lv_obj_set_style_transform_angle(a, -450, 0);

    lv_obj_t* b = lv_obj_create(parent);
    lv_obj_set_size(b, 8, 16);
    lv_obj_set_pos(b, 18, 9);
    clearStyle(b);
    setBg(b, color, LV_OPA_COVER);
    lv_obj_set_style_transform_angle(b, -450, 0);
}

void MusicPlayerScreen::create()
{
    lv_obj_t* screen = lv_scr_act();
    lv_obj_clean(screen);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    setBg(screen, kBg0, LV_OPA_COVER);
    lv_obj_set_style_pad_all(screen, 0, 0);

    lv_obj_t* bg = lv_obj_create(screen);
    lv_obj_set_size(bg, kScreenW, kScreenH);
    lv_obj_set_pos(bg, 0, 0);
    clearStyle(bg);
    setBg(bg, kBg1, LV_OPA_COVER);

    lv_obj_t* glow = lv_obj_create(bg);
    lv_obj_set_size(glow, 520, 172);
    lv_obj_set_pos(glow, -80, 0);
    clearStyle(glow);
    setBg(glow, kCoverA, LV_OPA_30);
    lv_obj_set_style_radius(glow, 40, 0);

    lv_obj_t* cover = lv_obj_create(bg);
    lv_obj_set_size(cover, 130, 130);
    lv_obj_set_pos(cover, 12, 21);
    clearStyle(cover);
    lv_obj_set_style_radius(cover, 12, 0);
    setBg(cover, kCoverB, LV_OPA_COVER);
    lv_obj_set_style_border_width(cover, 1, 0);
    lv_obj_set_style_border_color(cover, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_opa(cover, LV_OPA_30, 0);

    lv_obj_t* cover_band = lv_obj_create(cover);
    lv_obj_set_size(cover_band, 130, 38);
    lv_obj_set_pos(cover_band, 0, 92);
    clearStyle(cover_band);
    setBg(cover_band, kCoverC, LV_OPA_80);

    lv_obj_t* panel = makePanel(bg, 156, 16, 342, 140, kGlass, LV_OPA_70);

    title_ = makeLabel(panel, state_.title, &lv_font_montserrat_28, kText);
    lv_obj_set_pos(title_, 14, 14);
    lv_obj_set_size(title_, 312, 34);
    lv_label_set_long_mode(title_, LV_LABEL_LONG_DOT);

    subtitle_ = makeLabel(panel, "", &lv_font_montserrat_16, kMuted);
    lv_obj_set_pos(subtitle_, 14, 50);
    lv_obj_set_size(subtitle_, 312, 22);
    lv_label_set_long_mode(subtitle_, LV_LABEL_LONG_DOT);

    lv_obj_t* track = lv_obj_create(panel);
    lv_obj_set_size(track, 308, 6);
    lv_obj_set_pos(track, 14, 90);
    clearStyle(track);
    lv_obj_set_style_radius(track, LV_RADIUS_CIRCLE, 0);
    setBg(track, 0xffffff, LV_OPA_20);

    progress_ = lv_obj_create(track);
    lv_obj_set_size(progress_, 1, 6);
    lv_obj_set_pos(progress_, 0, 0);
    clearStyle(progress_);
    lv_obj_set_style_radius(progress_, LV_RADIUS_CIRCLE, 0);
    setBg(progress_, 0xffffff, LV_OPA_90);

    elapsed_ = makeLabel(panel, "0:00", &lv_font_montserrat_12, kMuted);
    lv_obj_set_pos(elapsed_, 14, 101);
    lv_obj_set_size(elapsed_, 58, 18);

    duration_ = makeLabel(panel, "0:00", &lv_font_montserrat_12, kMuted);
    lv_obj_set_pos(duration_, 264, 101);
    lv_obj_set_size(duration_, 58, 18);
    lv_obj_set_style_text_align(duration_, LV_TEXT_ALIGN_RIGHT, 0);

    lv_obj_t* prev = makeRoundButton(bg, 516, 59, 34, false);
    makePrevIcon(prev, 0xffffff);

    lv_obj_t* pause = makeRoundButton(bg, 550, 53, 46, true);
    makePauseIcon(pause, 0x050507);

    lv_obj_t* next = makeRoundButton(bg, 596, 59, 34, false);
    makeNextIcon(next, 0xffffff);

    timer_ = lv_timer_create(onTimer, 1000, this);
    updateUi();
}

void MusicPlayerScreen::destroy()
{
    if (timer_) {
        lv_timer_del(timer_);
        timer_ = nullptr;
    }
    title_ = subtitle_ = progress_ = elapsed_ = duration_ = nullptr;
}

void MusicPlayerScreen::onTimer(lv_timer_t* t)
{
    static_cast<MusicPlayerScreen*>(t->user_data)->updateUi();
}

void MusicPlayerScreen::updateUi()
{
    char subtitle[96];
    snprintf(subtitle, sizeof(subtitle), "%s · %s", state_.artist, state_.album);
    lv_label_set_text(title_, state_.title);
    lv_label_set_text(subtitle_, subtitle);

    const uint8_t pct = musicProgressPercent(state_);
    lv_obj_set_width(progress_, 308 * pct / 100);

    lv_label_set_text(elapsed_, "2:20");
    lv_label_set_text(duration_, "2:56");
}
```

- [ ] **Step 3: Add `music_state.cpp` to firmware sources**

This is done in Task 5 with all CMake updates.

---

### Task 4: Screen Manager and Swipe Switching

**Files:**
- Create: `main/screen_manager.h`
- Create: `main/screen_manager.cpp`

- [ ] **Step 1: Add manager declaration**

Create `main/screen_manager.h`:

```cpp
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
    static void onGestureEvent(lv_event_t* e);

    void switchTo(ScreenId target);
    void handleSwipe(SwipeDirection swipe);

    ScreenId current_ = ScreenId::Clock;
    ClockFaceScreen clock_;
    MusicPlayerScreen music_;
};
```

- [ ] **Step 2: Implement manager**

Create `main/screen_manager.cpp`:

```cpp
#include "screen_manager.h"

#include "lvgl.h"

ScreenManager& ScreenManager::instance()
{
    static ScreenManager manager;
    return manager;
}

void ScreenManager::create()
{
    current_ = ScreenId::Clock;
    clock_.create();
    attachGestureHandler(lv_scr_act());
}

void ScreenManager::destroy()
{
    if (current_ == ScreenId::Clock) {
        clock_.destroy();
    } else {
        music_.destroy();
    }
}

void ScreenManager::attachGestureHandler(lv_obj_t* root)
{
    lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(root, onGestureEvent, LV_EVENT_GESTURE, this);
}

void ScreenManager::onGestureEvent(lv_event_t* e)
{
    auto* manager = static_cast<ScreenManager*>(lv_event_get_user_data(e));
    const lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_LEFT) {
        manager->handleSwipe(SwipeDirection::Left);
    } else if (dir == LV_DIR_RIGHT) {
        manager->handleSwipe(SwipeDirection::Right);
    }
}

void ScreenManager::handleSwipe(SwipeDirection swipe)
{
    const ScreenId target = nextScreenForSwipe(current_, swipe);
    if (target != current_) {
        switchTo(target);
    }
}

void ScreenManager::switchTo(ScreenId target)
{
    if (target == current_) return;

    if (current_ == ScreenId::Clock) {
        clock_.destroy();
    } else {
        music_.destroy();
    }

    if (target == ScreenId::Clock) {
        clock_.create();
    } else {
        music_.create();
    }

    current_ = target;
    attachGestureHandler(lv_scr_act());
}
```

- [ ] **Step 3: Verify LVGL gesture direction at build time**

Run:

```bash
rg -n "LV_EVENT_GESTURE|lv_indev_get_gesture_dir|LV_DIR_LEFT" managed_components components main
```

Expected: LVGL headers expose these symbols. If the search only finds this new code and build later fails, replace this task's implementation with `SwipeGestureDetector` fed from `LV_EVENT_PRESSED` / `LV_EVENT_RELEASED` using `lv_indev_get_point()`.

---

### Task 5: Wire Into App and Build

**Files:**
- Modify: `main/main.cpp`
- Modify: `main/CMakeLists.txt`

- [ ] **Step 1: Update app entrypoint**

Modify `main/main.cpp` includes:

```cpp
#include "screen_manager.h"
```

Remove:

```cpp
#include "clock_face_screen.h"
```

Replace the final screen creation block:

```cpp
static ClockFaceScreen face;
if (LvglPort::Guard g; g) {
    face.create();
}
```

with:

```cpp
if (LvglPort::Guard g; g) {
    ScreenManager::instance().create();
}
```

- [ ] **Step 2: Update firmware source list**

Modify `main/CMakeLists.txt` `SRCS` to include:

```cmake
       "screen_nav.cpp"
       "music_state.cpp"
       "music_player_screen.cpp"
       "screen_manager.cpp"
```

The full source block should include:

```cmake
idf_component_register(
  SRCS "main.cpp"
       "lvgl_port.cpp"
       "touch_drv.cpp"
       "power_mgr.cpp"
       "clock_face_screen.cpp"
       "clock_net.cpp"
       "screen_nav.cpp"
       "music_state.cpp"
       "music_player_screen.cpp"
       "screen_manager.cpp"
  REQUIRES i2c_bsp i2c_equipment adc_bsp lcd_bl_pwm_bsp
           esp_wifi esp_event esp_netif nvs_flash lwip
           esp_driver_usb_serial_jtag esp_timer esp_lcd driver
           SensorLib esp_lcd_axs15231b
  INCLUDE_DIRS ".")
```

- [ ] **Step 3: Run pure logic tests**

Run:

```bash
c++ -std=c++17 -I. tests/test_screen_nav.cpp -o /tmp/test_screen_nav && /tmp/test_screen_nav
c++ -std=c++17 -I. tests/test_music_state.cpp -o /tmp/test_music_state && /tmp/test_music_state
```

Expected: both commands exit `0`.

- [ ] **Step 4: Build firmware**

Run with the repo's ESP-IDF environment:

```bash
export IDF_COMPONENT_CACHE_PATH=/workspace/espressif-tools/cache/ComponentManager
source /workspace/activate_idf_v6.0.1.sh
idf.py build
```

Expected: build completes successfully.

If `/workspace/activate_idf_v6.0.1.sh` is not present on this machine, report that firmware build could not be run locally and provide the pure test results.

---

### Task 6: Hardware Verification

**Files:**
- No code changes unless verification exposes a bug.

- [ ] **Step 1: Flash**

Run:

```bash
export IDF_COMPONENT_CACHE_PATH=/workspace/espressif-tools/cache/ComponentManager
source /workspace/activate_idf_v6.0.1.sh
idf.py -p /dev/cu.usbmodem111401 flash
```

Expected: firmware flashes successfully. Adjust the port if the board appears under a different device path.

- [ ] **Step 2: Verify startup screen**

Expected: device boots to the existing clock page. Time, battery, WiFi, and sync indicators still render.

- [ ] **Step 3: Verify navigation**

Expected:

- Left swipe on clock switches to the music player.
- Right swipe on music switches back to the clock.
- Tapping controls on music does not switch pages.
- Small horizontal movement while tapping does not switch pages.

- [ ] **Step 4: Verify music layout**

Expected:

- Placeholder cover block appears on the left.
- Glass-style info panel appears in the center.
- Previous, pause, next controls appear on the right.
- Text does not overlap controls.
- Progress bar fits inside the panel.

---

## Self-Review

Spec coverage:

- Default clock screen: Task 4 and Task 5.
- Left/right swipe navigation: Task 1 and Task 4.
- Liquid Glass music UI: Task 3.
- State model for future MQTT: Task 2.
- 640x172 fit: Task 3 and Task 6.
- MQTT ingestion and real cover rendering: explicitly deferred to the next implementation pass after static UI/navigation verification.

Placeholder scan:

- No incomplete implementation steps remain.

Type consistency:

- `ScreenId`, `SwipeDirection`, and `MusicState` names match across tasks.
- `musicProgressPercent()` is declared in `music_state.h`, defined in `music_state.cpp`, and used by `MusicPlayerScreen`.
