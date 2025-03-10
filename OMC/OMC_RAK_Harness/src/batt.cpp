#include "batt.h"

/**
 * @file batt.cpp
 * @brief Implementation of battery measurement functions for the OzarkMountainCat project.
 *
 * This file implements functions for initializing the ADC for battery voltage measurement,
 * reading the battery voltage, and converting the measured voltage (in millivolts) to a percentage.
 */

/**
 * @brief Initializes the battery measurement module.
 *
 * This function configures the ADC to use an internal 3.0V reference instead of the default 3.6V,
 * sets the ADC resolution to 12 bits (range 0..4095), and allows a short delay for the ADC to settle.
 */
void BattHandler::begin() {
  // Set the analog reference voltage to 3.0V (default is 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the analog read resolution to 12 bits (can be 8, 10, 12, or 14 bits)
  analogReadResolution(12);

  // Short delay to allow the ADC to stabilize after configuration
  delay(1);

  Serial.println("Battery measurement initialized.");
}

/**
 * @brief Reads the battery voltage in millivolts.
 *
 * This function reads the raw ADC value from the battery voltage pin (PIN_VBAT)
 * and multiplies it by REAL_VBAT_MV_PER_LSB to convert it to millivolts.
 *
 * @return float The measured battery voltage in millivolts.
 */
float BattHandler::readVBatt() {
    // Uncomment the following debug prints to see ADC conversion details.
    // Serial.print("MVOLTS_PER_LSB: ");
    // Serial.println(REAL_VBAT_MV_PER_LSB);
    // Serial.println(float(0.73242188F * 1.73));
    // Serial.print("VBAT_Analog_Read: ");
    // Serial.println(analogRead(PIN_VBAT));

    float vbat = analogRead(PIN_VBAT) * REAL_VBAT_MV_PER_LSB;
    return vbat;
}

/**
 * @brief Converts battery voltage in millivolts to a percentage.
 *
 * This function maps the battery voltage to a percentage value.
 * - Voltages >= 4200 mV correspond to 100%.
 * - Voltages <= 3050 mV correspond to 0%.
 * - Otherwise, a linear conversion is applied.
 *
 * @param mvolts The battery voltage in millivolts.
 * @return uint8_t The battery level as a percentage (0 to 100).
 */
uint8_t BattHandler::mvToPercent(float mvolts) {
    if (mvolts >= 4200) return 100;
    else if (mvolts <= 3050) return 0;
    else return (mvolts - 3000) / 12.0;
}
