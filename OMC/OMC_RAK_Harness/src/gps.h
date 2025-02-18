#pragma once
/**
 * @file gps.h
 * @brief Header file for the GPSHandler class.
 *
 * This file declares the GPSHandler class which provides an interface to the GNSS module
 * using the SparkFun u-blox GNSS Arduino Library. It contains functions to initialize the
 * module, update data, check fix status, and retrieve various GPS parameters.
 */

#include "main.h"
// #include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <SparkFun_u-blox_GNSS_v3.h>

/**
 * @class GPSHandler
 * @brief Manages GPS operations.
 *
 * The GPSHandler class encapsulates functions for initializing, updating, and retrieving
 * GPS data such as latitude, longitude, altitude, time, satellite count, and HDOP.
 */
class GPSHandler {
public:
    /**
     * @brief Default constructor.
     */
    GPSHandler() {}

    /**
     * @brief Initializes the GNSS module.
     *
     * Resets and configures the GNSS module, including setting power management mode.
     * If initialization fails, an error message is printed.
     *
     * @return true if initialization is successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Powers off the GNSS module.
     *
     * Turns off the GNSS module by manipulating the WB_IO2 pin.
     */
    void gpsOff();

    /**
     * @brief Updates the GPS data.
     *
     * Retrieves the latest data from the GNSS module. If a fix is available, updates
     * latitude, longitude, altitude, time, satellite count, and HDOP; otherwise, sets fix flag to false.
     */
    void update();

    /**
     * @brief Checks if a GPS fix is available.
     *
     * @return true if a valid GPS fix has been acquired, false otherwise.
     */
    bool hasFix();

    /**
     * @brief Retrieves the current latitude.
     *
     * @return The latitude as a double.
     */
    double getLatitude();

    /**
     * @brief Retrieves the current longitude.
     *
     * @return The longitude as a double.
     */
    double getLongitude();

    /**
     * @brief Retrieves the current altitude.
     *
     * @return The altitude as an integer.
     */
    int getAltitude();

    /**
     * @brief Retrieves the current hour.
     *
     * @return The hour (0-23) as a uint8_t.
     */
    uint8_t getHour();

    /**
     * @brief Retrieves the current minute.
     *
     * @return The minute (0-59) as a uint8_t.
     */
    uint8_t getMinute();

    /**
     * @brief Retrieves the current second.
     *
     * @return The second (0-59) as a uint8_t.
     */
    uint8_t getSecond();

    /**
     * @brief Retrieves the number of satellites in view.
     *
     * @return The satellite in view count as a uint8_t.
     */
    uint8_t getSIV();

    /**
     * @brief Retrieves the Horizontal Dilution of Precision (HDOP).
     *
     * @return The HDOP value as a uint16_t.
     */
    uint16_t getHDOP();

private:
    /**
     * @brief GNSS object from the SparkFun u-blox GNSS library.
     *
     * Used to interact with the GNSS module.
     */
    SFE_UBLOX_GNSS myGNSS;

    /**
     * @brief Flag indicating if a valid GPS fix is available.
     */
    bool fix = false;

    /**
     * @brief Latitude value.
     */
    double lat = 0.0;

    /**
     * @brief Longitude value.
     */
    double lon = 0.0;

    /**
     * @brief Altitude value.
     */
    double alt = 0;

    /**
     * @brief Current hour.
     */
    uint8_t hour = 0;

    /**
     * @brief Current minute.
     */
    uint8_t min = 0;

    /**
     * @brief Current second.
     */
    uint8_t sec = 0;

    /**
     * @brief Number of satellites in view.
     */
    uint8_t siv = 0;

    /**
     * @brief Horizontal Dilution of Precision.
     */
    uint16_t hdop = 0;
};
