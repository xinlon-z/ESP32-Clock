#include "../main/app/features/music/music_presenter.h"
#include "../main/app/core/event/event_bus.h"
#include "../main/app/services/mqtt_service.h"
#include "../main/app/services/cover_service.h"

#include "esp_heap_caps.h"

#include <stdio.h>

namespace {
int render_count = 0;
int cover_placeholder_count = 0;

int expect(bool condition, const char* message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}
} // namespace

namespace MusicMqtt {
struct CoverImage {
    uint8_t* data = nullptr;
    uint32_t size = 0;
};

void init() {}
bool takeCover(CoverImage*) { return false; }
} // namespace MusicMqtt

void MusicView::create() {}
void MusicView::destroy() {}
void MusicView::render(const MusicDisplayState&) { ++render_count; }
void MusicView::renderCover(const BorrowedCover&) {}
void MusicView::renderCoverPlaceholder() { ++cover_placeholder_count; }

int main()
{
    int failures = 0;
    EventBus::get().resetForTest();
    CoverService::get().clear();

    failures += expect(MqttService::get().applyField("title", "Queued", 6),
                       "initial music update should publish an event");

    MusicView view;
    MusicPresenter presenter(view);
    presenter.start();

    AppEvent event{};
    failures += expect(EventBus::get().poll(&event),
                       "start should not consume queued music event");
    failures += expect(event.type == AppEventType::MusicStateChanged,
                       "queued event should remain a music event");

    failures += expect(MqttService::get().applyField("title", "Ticked", 6),
                       "second music update should publish an event");
    presenter.tick();
    failures += expect(!EventBus::get().poll(&event),
                       "tick should consume queued presenter events");
    failures += expect(render_count >= 2, "presenter should render on start and tick");
    failures += expect(cover_placeholder_count >= 1, "start should render cover snapshot");

    presenter.stop();
    CoverService::get().clear();
    return failures == 0 ? 0 : 1;
}
