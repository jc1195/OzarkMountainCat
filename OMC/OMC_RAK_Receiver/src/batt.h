#pragma once

#include "main.h"

class BattHandler {
    public:
        BattHandler() {}
        void begin();
        float readVBatt();
        uint8_t mvToPercent(float mvolts);

    private:
        

};