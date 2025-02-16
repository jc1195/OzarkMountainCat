#include "gps.h"

bool GPSHandler::begin() {

    pinMode(WB_IO2, OUTPUT);
    digitalWrite(WB_IO2, 0);
    delay(100);
    digitalWrite(WB_IO2, 1);
    delay(100);

    if (myGNSS.begin() == false) {
        Serial.println("Failed to initialize GNSS!");
        return false;
    }
    // if (myGNSS.setHighPrecisionMode(true) == false) {
    //     Serial.println("Failed to set high precision mode!");
    //     return false;
    // }
    if (myGNSS.setPowerManagement(SFE_UBLOX_PMS_MODE_FULLPOWER) == false) {
        Serial.println("Failed to set power management!");
        return false;
    }

    myGNSS.saveConfiguration();

    // pinMode(LED_BLUE, OUTPUT);
    // digitalWrite(LED_BLUE, HIGH);
    Serial.println("GNSS initialized.");
    return true;
}

void GPSHandler::gpsOff() {
    digitalWrite(WB_IO2, 0);
    delay(100);
    digitalWrite(WB_IO2, 1);
    delay(100);
    digitalWrite(WB_IO2, 0);
    delay(100);
}

void GPSHandler::update() {
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
        alt = (myGNSS.getAltitudeMSL() / 1000.0) * 3.28084;
    } else {
        fix = false;
    }
}

bool GPSHandler::hasFix() {
    return fix;
}

int GPSHandler::getAltitude() {
    return alt;
}

double GPSHandler::getLatitude() {
    return lat;
}

double GPSHandler::getLongitude() {
    return lon;
}

uint8_t GPSHandler::getHour() {
    return hour;
}

uint8_t GPSHandler::getMinute() {
    return min;
}

uint8_t GPSHandler::getSecond() {
    return sec;
}

uint8_t GPSHandler::getSIV() {
    return siv;
}

uint16_t GPSHandler::getHDOP() {
    return hdop;
}
