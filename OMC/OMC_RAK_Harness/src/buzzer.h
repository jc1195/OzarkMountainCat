#pragma once
/**
 * @file buzzer.h
 * @brief Header file for buzzer functionality.
 *
 * This file defines musical note frequencies, note duration macros, and the
 * BuzzerHandler class which provides functions to initialize and control the buzzer.
 */

#include "main.h"

/**
 * @name Note Frequency Macros (High, Middle, and Low Registers)
 * 
 * These macros define note frequencies (in Hz) for different octaves.
 * Negative values indicate a rest or a special note (e.g., NTC0).
 * @{
 */
#define NTC0 -1      /**< Represents a rest or an invalid note. */
#define NTC1 262     /**< Frequency for note C4 (middle C) in Hz. */
#define NTC2 294     /**< Frequency for note D4 in Hz. */
#define NTC3 330     /**< Frequency for note E4 in Hz. */
#define NTC4 350     /**< Frequency for note F4 in Hz. */
#define NTC5 393     /**< Frequency for note G4 in Hz. */
#define NTC6 441     /**< Frequency for note A4 in Hz. */
#define NTC7 495     /**< Frequency for note B4 in Hz. */
/** @} */

/**
 * @name Note Frequency Macros for Lower Octave
 * @{
 */
#define NTCL1 131    /**< Frequency for note C3 in Hz. */
#define NTCL2 147    /**< Frequency for note D3 in Hz. */
#define NTCL3 165    /**< Frequency for note E3 in Hz. */
#define NTCL4 175    /**< Frequency for note F3 in Hz. */
#define NTCL5 196    /**< Frequency for note G3 in Hz. */
#define NTCL6 221    /**< Frequency for note A3 in Hz. */
#define NTCL7 248    /**< Frequency for note B3 in Hz. */
/** @} */

/**
 * @name Note Frequency Macros for Higher Octave
 * @{
 */
#define NTCH1 525    /**< Frequency for note C5 in Hz. */
#define NTCH2 589    /**< Frequency for note D5 in Hz. */
#define NTCH3 661    /**< Frequency for note E5 in Hz. */
#define NTCH4 700    /**< Frequency for note F5 in Hz. */
#define NTCH5 786    /**< Frequency for note G5 in Hz. */
#define NTCH6 882    /**< Frequency for note A5 in Hz. */
#define NTCH7 990    /**< Frequency for note B5 in Hz. */
/** @} */

/**
 * @name Note Duration Macros
 * 
 * These macros define the relative duration of musical notes.
 * Values represent multipliers for a base duration (e.g., 500 ms) to create whole, half,
 * quarter, eighth, and sixteenth notes.
 * @{
 */
#define WHOLE 1        /**< Whole note duration multiplier. */
#define HALF 0.5       /**< Half note duration multiplier. */
#define QUARTER 0.25   /**< Quarter note duration multiplier. */
#define EIGHTH 0.125   /**< Eighth note duration multiplier. */
#define SIXTEENTH 0.0625 /**< Sixteenth note duration multiplier. */
/** @} */

/**
 * @class BuzzerHandler
 * @brief Provides functions for controlling the buzzer.
 *
 * The BuzzerHandler class offers functions to initialize the buzzer hardware,
 * play a predefined tune, and turn the buzzer off. It also maintains an internal
 * state indicating whether the buzzer is currently on.
 */
class BuzzerHandler {
    public:
        /**
         * @brief Default constructor.
         */
        BuzzerHandler() {}

        /**
         * @brief Initializes the buzzer.
         *
         * Configures the buzzer pin as an output and calculates the length of the tune array.
         */
        void begin();

        /**
         * @brief Plays the tune on the buzzer.
         *
         * Iterates through a predefined sequence of notes (stored in the global 'tune' array)
         * and plays them with durations specified by the 'durt' array. A short vTaskDelay is used
         * to yield control between notes. After the tune is played, the buzzer is turned off.
         */
        void on();

        /**
         * @brief Turns off the buzzer.
         *
         * Sets the buzzer pin to LOW and updates the internal state.
         */
        void off();

        /**
         * @brief Checks if the buzzer is currently active.
         *
         * @return true if the buzzer is on, false otherwise.
         */
        bool isOn();

    private:
        /**
         * @brief Internal flag indicating if the buzzer is active.
         */
        bool buzzerOn = false;
};
