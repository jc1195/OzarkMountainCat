#pragma once
/**
 * @file batt.h
 * @brief Header file for the BattHandler class.
 *
 * This file declares the BattHandler class, which provides functions to
 * initialize battery measurement, read battery voltage, and convert the measured
 * voltage (in millivolts) to a percentage.
 */

#include "main.h"

/**
 * @class BattHandler
 * @brief Handles battery voltage measurement and conversion.
 *
 * The BattHandler class provides methods to initialize the ADC for battery measurement,
 * read the battery voltage in millivolts, and convert that value to a percentage.
 */
class BattHandler {
    public:
        /**
         * @brief Default constructor for BattHandler.
         */
        BattHandler() {}

        /**
         * @brief Initializes the battery measurement module.
         *
         * This function sets the analog reference to 3.0V, configures the ADC resolution to 12 bits,
         * and allows a short delay for the ADC to settle.
         */
        void begin();

        /**
         * @brief Reads the battery voltage.
         *
         * This function reads the raw ADC value from the battery voltage pin and converts it to millivolts.
         *
         * @return float The measured battery voltage in millivolts.
         */
        float readVBatt();

        /**
         * @brief Converts battery voltage in millivolts to a percentage.
         *
         * If the battery voltage is greater than or equal to 4200 mV, returns 100%.
         * If the voltage is less than or equal to 3050 mV, returns 0%.
         * Otherwise, returns a linear mapping from millivolts to percentage.
         *
         * @param mvolts The battery voltage in millivolts.
         * @return uint8_t The battery level as a percentage (0 to 100).
         */
        uint8_t mvToPercent(float mvolts);

    private:
        // No private members declared.
};
