#include "rgb.h"

/**
 * @brief Constructor for RGBHandler.
 *
 * Initializes the NeoPixel object using NUM_PIXELS and PIN_RGB.
 * The NeoPixel is configured with NEO_GRB color order and an 800 KHz signal.
 */
RGBHandler::RGBHandler() : strip(NUM_PIXELS, PIN_RGB, NEO_GRB + NEO_KHZ800) {
    // Constructor body can remain empty.
}

/**
 * @brief Initializes the RGB LED.
 *
 * Calls the begin() method on the NeoPixel object and immediately updates the display
 * (which clears the LED to 'off').
 */
void RGBHandler::begin() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    delay(10);
    off();
    Serial.println("RGB LED initialized.");
    
}

/**
 * @brief Sets the color of the RGB LED.
 *
 * This method sets the color of the first pixel on the NeoPixel strip to the specified RGB values.
 * It then updates the LED display with strip.show().
 *
 */
void RGBHandler::setColor() {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(receivedPacket.r, receivedPacket.g, receivedPacket.b));
    }
    strip.show();
}

/**
 * @brief Turns off the RGB LED.
 *
 * Sets the color of the first pixel to off (0, 0, 0) and updates the display.
 */
void RGBHandler::off() {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}
