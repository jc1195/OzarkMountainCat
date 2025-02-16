#pragma once

#include "main.h"

class GPSHandler {
    public:
        GPSHandler() {}
        bool begin();
        void gpsOff();
        void update();
        bool hasFix();
        double getLatitude();
        double getLongitude();
        int getAltitude();
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
        double alt = 0;
        uint8_t hour = 0;
        uint8_t min = 0;
        uint8_t sec = 0;
        uint8_t siv = 0;
        uint16_t hdop = 0;

    };