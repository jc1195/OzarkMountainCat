#pragma once
/**
 * @file rgb.h
 * @brief Header file for RGB LED control using the Adafruit NeoPixel library.
 *
 * This file defines the RGBHandler class, which provides methods to initialize,
 * control, and turn off the Adafruit Flora RGB Smart NeoPixel.
 */

#include "main.h"

/**
 * @brief Define the data pin for the NeoPixel.
 *
 * For the RAK19003 baseboard, the NeoPixel is connected to the RX1 point.
 * If RX1 is not defined in main.h, define it here (replace 9 with the appropriate pin number).
 */
#ifndef PIN_RGB
#define PIN_RGB PIN_SERIAL1_TX  // Replace with your pin number if necessary
#endif

/**
 * @brief Define the number of pixels.
 *
 * For a single Adafruit Flora RGB Smart NeoPixel, this is 1.
 */
#ifndef NUM_PIXELS
#define NUM_PIXELS 2
#endif

/**
 * @class RGBHandler
 * @brief Class to control the Adafruit Flora RGB Smart NeoPixel.
 *
 * This class uses the Adafruit NeoPixel library to control an RGB LED.
 * It provides methods to initialize the LED, set its color, and turn it off.
 */
class RGBHandler {
public:
    /**
     * @brief Constructor for the RGBHandler class.
     *
     * Initializes the internal Adafruit_NeoPixel object with the specified number of pixels
     * and data pin. The NeoPixel is configured for GRB color ordering with an 800 KHz signal.
     */
    RGBHandler();

    /**
     * @brief Initializes the NeoPixel LED.
     *
     * Must be called in the setup() function to initialize the LED and clear any previous color.
     */
    void begin();

    /**
     * @brief Sets the color of the NeoPixel LED.
     *
     * Sets the first (and only) pixel of the NeoPixel strip to the specified RGB color.
     *
     */
    void setColor();

    /**
     * @brief Turns off the NeoPixel LED.
     *
     * Sets the pixel color to off (black) and updates the display.
     */
    void off();

private:
    /** @brief Instance of the Adafruit_NeoPixel library to control the LED. */
    Adafruit_NeoPixel strip;
};
