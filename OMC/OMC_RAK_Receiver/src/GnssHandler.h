#pragma once
#include "main.h"

class GnssHandler {
public:
    GnssHandler() {}
    bool begin();
    void update();
    bool hasFix();
    double getLatitude();
    double getLongitude();
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
    uint8_t getSIV();
    uint16_t getHDOP();

private:
    SFE_UBLOX_GNSS myGNSS;
    bool fix = false;
    double lat = 0.0;
    double lon = 0.0;
    uint8_t hour = 0;
    uint8_t min = 0;
    uint8_t sec = 0;
    uint8_t siv = 0;
    uint16_t hdop = 0;
};
