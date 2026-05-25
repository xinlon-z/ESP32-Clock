#include "clock_presenter.h"

#include "app/core/event/event_bus.h"
#include "app/services/network_service.h"
#include "app/services/power_service.h"
#include "app/services/time_service.h"

ClockPresenter::ClockPresenter(ClockView& view) : view_(view) {}

void ClockPresenter::start()
{
    running_ = true;
    model_.resetBattery();

    TimeService::get().poll();
    PowerService::get().poll();
    NetworkService::get().poll();

    AppEvent event{};
    while (EventBus::get().poll(&event)) {
        if (event.type == AppEventType::ClockTimeChanged) {
            last_time_revision_ = event.payload.clock_time.revision;
        } else if (event.type == AppEventType::PowerStateChanged) {
            last_power_revision_ = event.payload.power_state.revision;
        } else if (event.type == AppEventType::NetworkStateChanged) {
            last_network_revision_ = event.payload.network_state.revision;
        }
    }

    renderAll();
}

void ClockPresenter::stop()
{
    running_ = false;
}

void ClockPresenter::tick()
{
    if (!running_) {
        return;
    }

    TimeService::get().poll();
    PowerService::get().poll();
    NetworkService::get().poll();

    bool time_changed = false;
    bool power_changed = false;
    bool network_changed = false;
    AppEvent event{};
    while (EventBus::get().poll(&event)) {
        if (event.type == AppEventType::ClockTimeChanged &&
            event.payload.clock_time.revision != last_time_revision_) {
            last_time_revision_ = event.payload.clock_time.revision;
            time_changed = true;
        } else if (event.type == AppEventType::PowerStateChanged &&
                   event.payload.power_state.revision != last_power_revision_) {
            last_power_revision_ = event.payload.power_state.revision;
            power_changed = true;
        } else if (event.type == AppEventType::NetworkStateChanged &&
                   event.payload.network_state.revision != last_network_revision_) {
            last_network_revision_ = event.payload.network_state.revision;
            network_changed = true;
        }
    }

    if (power_changed) {
        renderPowerSnapshot();
    }
    if (time_changed) {
        renderTimeSnapshot();
    }
    if (network_changed) {
        renderNetworkSnapshot();
    }
}

void ClockPresenter::renderAll()
{
    renderPowerSnapshot();
    renderTimeSnapshot();
    renderNetworkSnapshot();
}

void ClockPresenter::renderTimeSnapshot()
{
    const ClockSnapshot clock = TimeService::get().snapshot();
    const ClockDisplayState time = model_.buildTime(clock.rtc_ok, clock.hour, clock.minute, clock.second,
                                                    clock.week, clock.month, clock.day);
    view_.renderTime(time, dimmed_);
}

void ClockPresenter::renderPowerSnapshot()
{
    const PowerSnapshot power = PowerService::get().snapshot();
    dimmed_ = power.dimmed;
    external_power_ = power.external_power;

    BatteryDisplayState battery = model_.buildBattery(power.battery_percent);
    view_.renderBattery(battery, dimmed_);
}

void ClockPresenter::renderNetworkSnapshot()
{
    const NetworkSnapshot network = NetworkService::get().snapshot();
    NetworkDisplayState net{};
    net.wifi_connected = network.wifi_connected;
    net.sync_in_progress = network.sync_in_progress;
    net.ntp_synced = network.ntp_synced;
    net.external_power = external_power_;
    view_.renderNetwork(net, dimmed_);
}
