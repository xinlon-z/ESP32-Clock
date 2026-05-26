#include "app/services/power_service.cpp"
#include "app/core/event/event_bus.cpp"

#include <stdio.h>

int main()
{
    PowerSnapshot snapshot{};
    snapshot.external_power = true;
    snapshot.battery_percent = 87;
    snapshot.dimmed = true;
    snapshot.sleeping = false;
    snapshot.revision = 5;

    if (!snapshot.external_power || snapshot.battery_percent != 87 || !snapshot.dimmed || snapshot.sleeping) {
        printf("snapshot fields failed\n");
        return 1;
    }
    if (snapshot.revision != 5) {
        printf("revision failed\n");
        return 1;
    }
    return 0;
}
