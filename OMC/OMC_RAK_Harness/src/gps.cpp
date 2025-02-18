#include "gps.h"

/**
 * @brief Initializes the GNSS module.
 *
 * This function resets and initializes the GNSS hardware. It sets the power
 * management mode and saves the configuration. If any step fails, it prints an error
 * message and returns false.
 *
 * @return true if initialization is successful, false otherwise.
 */
bool GPSHandler::begin() {
    // Reset the GNSS module via WB_IO2 pin.
    pinMode(WB_IO2, OUTPUT);
    digitalWrite(WB_IO2, 0);
    delay(100);
    digitalWrite(WB_IO2, 1);
    delay(100);

    

    // Initialize GNSS using the SparkFun library.
    if (myGNSS.begin() == false) {
        //Serial.println("Failed to initialize GNSS!");
        return false;
    }
    // Optional: set high precision mode (commented out).
    // if (myGNSS.setHighPrecisionMode(true) == false) {
    //     //Serial.println("Failed to set high precision mode!");
    //     return false;
    // }
    // Set the power management mode to full power.
    // if (myGNSS.setPowerManagement(SFE_UBLOX_PMS_MODE_FULLPOWER) == false) {
    //     //Serial.println("Failed to set power management!");
    //     return false;
    // }

    // // Save the GNSS configuration.
    // myGNSS.saveConfiguration();

    // Uncomment if using a LED indicator for GNSS status.
    // pinMode(LED_BLUE, OUTPUT);
    // digitalWrite(LED_BLUE, HIGH);
    //Serial.println("GNSS initialized.");
    return true;
}

/**
 * @brief Powers off the GNSS module.
 *
 * This function manipulates the WB_IO2 pin to turn off the GNSS module.
 */
void GPSHandler::gpsOff() {
    digitalWrite(WB_IO2, 0);
    delay(100);
    digitalWrite(WB_IO2, 1);
    delay(100);
    digitalWrite(WB_IO2, 0);
    delay(100);
}

/**
 * @brief Updates the GNSS data.
 *
 * This function checks if a GNSS fix is available and, if so, updates global
 * variables with the latest GNSS data (latitude, longitude, time, satellite count, HDOP, and altitude).
 * If no fix is available, it sets the fix flag to false.
 */
void GPSHandler::update() {
    // Update GNSS data using the SparkFun library.
    ////Serial.println("Updating GPS data...");
    if (myGNSS.getGnssFixOk()) {
        fix = true;
        // Convert raw latitude and longitude values.
        lat = myGNSS.getLatitude() / 10000000.0;
        lon = myGNSS.getLongitude() / 10000000.0;
        hour = myGNSS.getHour();
        min = myGNSS.getMinute();
        sec = myGNSS.getSecond();
        siv = myGNSS.getSIV();
        hdop = myGNSS.getHorizontalDOP();
        // Convert altitude from meters to feet: (meters/1000)*3.28084.
        alt = (myGNSS.getAltitudeMSL() / 1000.0) * 3.28084;
    } else {
        fix = false;
    }
}

/**
 * @brief Checks if a GNSS fix is available.
 *
 * @return true if a GNSS fix is available, false otherwise.
 */
bool GPSHandler::hasFix() {
    return fix;
}

/**
 * @brief Gets the altitude.
 *
 * @return Altitude value.
 */
int GPSHandler::getAltitude() {
    return alt;
}

/**
 * @brief Gets the latitude.
 *
 * @return Latitude value.
 */
double GPSHandler::getLatitude() {
    return lat;
}

/**
 * @brief Gets the longitude.
 *
 * @return Longitude value.
 */
double GPSHandler::getLongitude() {
    return lon;
}

/**
 * @brief Gets the hour from the GNSS data.
 *
 * @return Hour value.
 */
uint8_t GPSHandler::getHour() {
    return hour;
}

/**
 * @brief Gets the minute from the GNSS data.
 *
 * @return Minute value.
 */
uint8_t GPSHandler::getMinute() {
    return min;
}

/**
 * @brief Gets the second from the GNSS data.
 *
 * @return Second value.
 */
uint8_t GPSHandler::getSecond() {
    return sec;
}

/**
 * @brief Gets the number of satellites in view.
 *
 * @return Satellite in view (SIV) count.
 */
uint8_t GPSHandler::getSIV() {
    return siv;
}

/**
 * @brief Gets the Horizontal Dilution of Precision (HDOP).
 *
 * @return HDOP value.
 */
uint16_t GPSHandler::getHDOP() {
    return hdop;
}
