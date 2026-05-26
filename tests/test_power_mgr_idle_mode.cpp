#define private public
#include "platform/power_mgr.cpp"
#undef private

#include <stdio.h>

extern "C" void adc_get_value(float* value, int* data)
{
    if (value) *value = 0.0f;
    if (data) *data = 0;
}

namespace ClockNet {
void pauseForSleep() {}
void requestSync() {}
} // namespace ClockNet

int main()
{
    struct Case {
        bool external_power;
        uint32_t idle_ms;
        PowerManager::IdleMode expected;
    };

    const Case cases[] = {
        {false, 29999, PowerManager::IdleMode::Active},
        {false, 30000, PowerManager::IdleMode::Dimmed},
        {false, 299999, PowerManager::IdleMode::Dimmed},
        {false, 300000, PowerManager::IdleMode::Sleeping},
        {true,  300000, PowerManager::IdleMode::Active},
        {false, 0,      PowerManager::IdleMode::Active},
    };

    for (const Case& c : cases) {
        const PowerManager::IdleMode actual =
            PowerManager::computeIdleMode(c.external_power, c.idle_ms);
        if (actual != c.expected) {
            printf("computeIdleMode(ext=%d, idle=%u) expected %d, got %d\n",
                   c.external_power,
                   static_cast<unsigned>(c.idle_ms),
                   static_cast<int>(c.expected),
                   static_cast<int>(actual));
            return 1;
        }
    }

    return 0;
}
