#include "app/features/clock/clock_model.cpp"

#include <stdio.h>
#include <string.h>

int main()
{
    ClockModel model;
    ClockDisplayState invalid = model.buildTime(false, 0, 0, 0, 0, 0, 0);
    if (strcmp(invalid.time, "--:--") != 0 || strcmp(invalid.weekday, "RTC") != 0 || strcmp(invalid.date, "--/--") != 0) {
        printf("invalid RTC display failed\n");
        return 1;
    }

    ClockDisplayState valid = model.buildTime(true, 9, 5, 2, 1, 5, 24);
    if (strcmp(valid.time, "09:05") != 0 || strcmp(valid.weekday, "Mon") != 0 || strcmp(valid.date, "05/24") != 0) {
        printf("valid RTC display failed: %s %s %s\n", valid.time, valid.weekday, valid.date);
        return 1;
    }

    ClockDisplayState blink = model.buildTime(true, 9, 5, 3, 1, 5, 24);
    if (strcmp(blink.time, "09 05") != 0) {
        printf("colon blink failed: %s\n", blink.time);
        return 1;
    }

    BatteryDisplayState first = model.buildBattery(50);
    if (first.percent != 50 || !first.update_label) {
        printf("battery first display failed: %d %d\n", first.percent, first.update_label);
        return 1;
    }

    BatteryDisplayState held = model.buildBattery(53);
    if (held.percent != 50 || held.update_label) {
        printf("battery hysteresis hold failed: %d %d\n", held.percent, held.update_label);
        return 1;
    }

    BatteryDisplayState changed = model.buildBattery(56);
    if (changed.percent != 56 || !changed.update_label) {
        printf("battery hysteresis update failed: %d %d\n", changed.percent, changed.update_label);
        return 1;
    }

    BatteryDisplayState unknown = model.buildBattery(-1);
    if (unknown.percent != -1 || !unknown.update_label) {
        printf("battery reset failed: %d %d\n", unknown.percent, unknown.update_label);
        return 1;
    }

    BatteryDisplayState after_reset = model.buildBattery(53);
    if (after_reset.percent != 53 || !after_reset.update_label) {
        printf("battery after reset failed: %d %d\n", after_reset.percent, after_reset.update_label);
        return 1;
    }
    return 0;
}
