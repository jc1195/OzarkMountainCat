#pragma once
/**
 * @file buzzer.cpp
 * @brief Implementation of buzzer functionality for the OzarkMountainCat project.
 *
 * This file implements functions for initializing the buzzer hardware, playing a predefined tune,
 * and turning off the buzzer.
 */

#include "buzzer.h"

/**
 * @brief Array of note frequencies for the tune.
 *
 * Contains a list of note frequencies (defined as macros like NTC5, NTCH1, etc.)
 * representing the musical notes to be played.
 */
int tune[] = // List the frequencies according to the spectrum
{
  NTC5, NTC5, NTC6,
  NTCH1, NTC6, NTC5, NTC6, NTCH1, NTC6, NTC5,
  NTC3, NTC3, NTC3, NTC5,
  NTC6, NTC6, NTC5, NTCH3, NTCH3, NTCH2, NTCH1,
  NTCH2, NTCH1, NTCH2,
  NTCH3, NTCH3, NTCH2, NTCH3, NTCH2, NTCH1, NTCH2, NTCH1, NTC6,

  NTCH2, NTCH2, NTCH2, NTCH1, NTC6, NTC5,
  NTC6, NTC5, NTC5, NTCH1, NTC6, NTC5, NTC1, NTC3,
  NTC2, NTC1, NTC2,
  NTC3, NTC5, NTC5, NTC3, NTCH1, NTC7,
  NTC6, NTC5, NTC6, NTCH1, NTCH2, NTCH3,

  NTCH3, NTCH2, NTCH1, NTC5, NTCH1, NTCH2, NTCH3,

  NTCH2, NTC0, NTCH3, NTCH2,
  NTCH1, NTC0, NTCH2, NTCH1, NTC6, NTC0,

  NTCH2, NTC6, NTCH1, NTCH1, NTCH1, NTC6, NTC5, NTC3,
  NTC5,
  NTC5, NTC6, NTCH1, NTC7, NTC6,
  NTCH3, NTCH3, NTCH3, NTCH3, NTCH2, NTCH2, NTCH1,
  NTC6, NTCH3, NTCH2, NTCH1, NTCH2, NTCH1, NTC6,
  NTCH1,
};

/**
 * @brief Array of note durations for the tune.
 *
 * Contains a list of beat values (using macros such as HALF, QUARTER, WHOLE, etc.)
 * that correspond to the duration for each note in the tune array.
 */
float durt[] = // List the beats according to the notation
{
  HALF, QUARTER, QUARTER,
  WHOLE + HALF, HALF, HALF, QUARTER, QUARTER, HALF, HALF,
  WHOLE + WHOLE + WHOLE, HALF, QUARTER, QUARTER,
  WHOLE + HALF, HALF, HALF, HALF, QUARTER, QUARTER, HALF,
  WHOLE + WHOLE + WHOLE, HALF, HALF,
  HALF, HALF, HALF, QUARTER, QUARTER, HALF, QUARTER, QUARTER, HALF,
  HALF, HALF, HALF, QUARTER, QUARTER, WHOLE + WHOLE,
  HALF, HALF, HALF, HALF, HALF, HALF, HALF, HALF,
  WHOLE + WHOLE + WHOLE, HALF, HALF,


  WHOLE + HALF, HALF, HALF, HALF, HALF, HALF,
  WHOLE + HALF, HALF, WHOLE, HALF, QUARTER, QUARTER,
  WHOLE + HALF, HALF, HALF, HALF, HALF, QUARTER, QUARTER,
  WHOLE + WHOLE + WHOLE, HALF, QUARTER, QUARTER,
  WHOLE, HALF, QUARTER, QUARTER, WHOLE, WHOLE,
  HALF, HALF, HALF, HALF, WHOLE, QUARTER, QUARTER, HALF,
  WHOLE + WHOLE + WHOLE + WHOLE,
  HALF, HALF, HALF, HALF, WHOLE + WHOLE,
  HALF, HALF, HALF, HALF, WHOLE + HALF, QUARTER, QUARTER,
  WHOLE + HALF, HALF, WHOLE, QUARTER, QUARTER, QUARTER, QUARTER, WHOLE + WHOLE + WHOLE + WHOLE,
};

/**
 * @brief Global variable to store the length of the tune.
 *
 * This variable is calculated in BuzzerHandler::begin() based on the size of the tune array.
 */
int length = 0;

/**
 * @brief Initializes the buzzer hardware.
 *
 * Configures the buzzer pin as an output and calculates the length of the tune array.
 */
void BuzzerHandler::begin() {
  // Serial.end(); // (Optional: stop Serial communication if needed)
  pinMode(PIN_BUZZER, OUTPUT);
  // Optionally add PWM support to the buzzer pin:
  // HwPWM0.addPin(PIN_BUZZER);
  length = sizeof(tune) / sizeof(tune[0]); // Calculate the number of notes.
}

/**
 * @brief Plays the buzzer tune.
 *
 * Activates the buzzer and plays a sequence of tones defined in the tune array.
 * For each note, it plays the tone, delays based on the beat duration (adjusted by a multiplier),
 * yields control for 10 ms using vTaskDelay, and then stops the tone.
 * After the tune is played, the buzzer is turned off.
 */
void BuzzerHandler::on() {
    buzzerOn = true;
    for (int x = 0; x < length; x++)
    {
      // Play the note at frequency specified by tune[x]
      tone(PIN_BUZZER, tune[x]);
      // Alternatively, you could use PWM:
      // HwPWM0.writePin(PIN_BUZZER, tune[x], false);
      // Delay for the duration of the note (multiplier 500 adjusts the beat)
      delay(500 * durt[x]);
      // Yield to allow other tasks to run (10 ms delay)
      vTaskDelay(pdMS_TO_TICKS(10));
      // Stop the tone
      noTone(PIN_BUZZER);
    }
    // After the tune is finished, turn off the buzzer.
    off();
}

/**
 * @brief Turns off the buzzer.
 *
 * Sets the buzzer pin to LOW and updates the internal state to indicate the buzzer is off.
 */
void BuzzerHandler::off() {
  digitalWrite(PIN_BUZZER, LOW);
  buzzerOn = false;
}

/**
 * @brief Checks if the buzzer is currently on.
 *
 * @return true if the buzzer is active, false otherwise.
 */
bool BuzzerHandler::isOn() {
  return buzzerOn;
}
