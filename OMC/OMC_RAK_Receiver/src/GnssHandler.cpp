#include "GnssHandler.h"

bool GnssHandler::begin() {
    Wire.begin();
    if (myGNSS.begin() == false) {
        Serial.println("Failed to initialize GNSS!");
        return false;
    }
    // Power save mode if desired:
    //myGNSS.setPowerSaveMode(true);
    return true;
}

void GnssHandler::update() {
    // Update GNSS data
    // This depends on the SparkFun library specifics
    // Just a placeholder logic:
    if (myGNSS.getGnssFixOk()) {
        fix = true;
        lat = myGNSS.getLatitude() / 10000000.0;
        lon = myGNSS.getLongitude() / 10000000.0;
        hour = myGNSS.getHour();
        min = myGNSS.getMinute();
        sec = myGNSS.getSecond();
        siv = myGNSS.getSIV();
        hdop = myGNSS.getHorizontalDOP();
    } else {
        fix = false;
    }
}

bool GnssHandler::hasFix() {
    return fix;
}

double GnssHandler::getLatitude() {
    return lat;
}

double GnssHandler::getLongitude() {
    return lon;
}

uint8_t GnssHandler::getHour() {
    return hour;
}

uint8_t GnssHandler::getMinute() {
    return min;
}

uint8_t GnssHandler::getSecond() {
    return sec;
}

uint8_t GnssHandler::getSIV() {
    return siv;
}

uint16_t GnssHandler::getHDOP() {
    return hdop;
}
