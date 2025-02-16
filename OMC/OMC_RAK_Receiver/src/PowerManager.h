#pragma once
#include "main.h"

class PowerManager {
public:
    void begin(DeviceMode initialMode) {
        currentMode = initialMode;
        // TODO: Setup RTC or other wake sources if needed
    }

    void setMode(DeviceMode mode) { currentMode = mode; }
    DeviceMode getMode() { return currentMode; }

private:
    DeviceMode currentMode;
};
