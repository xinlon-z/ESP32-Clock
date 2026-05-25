#include "cover_service.h"

#include "../core/event/event_bus.h"
#include "esp_heap_caps.h"

CoverService& CoverService::get()
{
    static CoverService service;
    return service;
}

uint32_t CoverService::acceptJpeg(uint8_t* data, uint32_t size)
{
    if (!data || size == 0) {
        return 0;
    }

    uint32_t cover_id = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        releaseActive();
        cover_id = ++next_cover_id_;
        active_.cover_id = cover_id;
        active_.status = CoverStatus::Loading;
        active_.jpeg_data = data;
        active_.jpeg_size = size;
    }

    publishChanged(cover_id, CoverStatus::Loading);
    return cover_id;
}

CoverBuffer CoverService::active()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

void CoverService::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    releaseActive();
    active_ = CoverBuffer{};
}

void CoverService::releaseActive()
{
    if (active_.jpeg_data) {
        heap_caps_free(active_.jpeg_data);
    }
    if (active_.pixels) {
        heap_caps_free(active_.pixels);
    }
    active_ = CoverBuffer{};
}

void CoverService::publishChanged(uint32_t cover_id, CoverStatus status)
{
    AppEvent event{};
    event.type = AppEventType::CoverStateChanged;
    event.payload.cover_state.cover_id = cover_id;
    event.payload.cover_state.status = status;
    EventBus::get().publish(event);
}
