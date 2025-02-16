#include "batt.h"

void BattHandler::begin() {
  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

   // Let the ADC settle
   delay(1);
}

float BattHandler::readVBatt() {
    // Serial.print("MVOLTS_PER_LSB: ");
    // Serial.println(REAL_VBAT_MV_PER_LSB);
    // Serial.println(float(0.73242188F * 1.73));
    // Serial.print("VBAT_Analog_Read: ");
    // Serial.println(analogRead(PIN_VBAT));

    float vbat = analogRead(PIN_VBAT) * REAL_VBAT_MV_PER_LSB;
    return vbat;
}

uint8_t BattHandler::mvToPercent(float mvolts) {
    if (mvolts >= 4200) return 100;
    else if (mvolts <= 3050) return 0;
    else return (mvolts - 3000) / 12.0;
}