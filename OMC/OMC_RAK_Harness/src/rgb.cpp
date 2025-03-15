#include "rgb.h"

// Helper function that maps a hue value (0-255) to an RGB color.
// It takes the hue and the NeoPixel strip instance to use the Color() method.
static uint32_t Wheel(uint8_t pos, Adafruit_NeoPixel &strip) {
    pos = 255 - pos;
    if (pos < 85) {
        return strip.Color(255 - pos * 3, 0, pos * 3);
    } else if (pos < 170) {
        pos -= 85;
        return strip.Color(0, pos * 3, 255 - pos * 3);
    } else {
        pos -= 170;
        return strip.Color(pos * 3, 255 - pos * 3, 0);
    }
}

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

void RGBHandler::rainbowCycle() {
    rainbowOn = true;  // Enable the rainbow effect
    
    // Set starting hues for each LED (different starting points for variation)
    uint8_t hue1 = 0;
    uint8_t hue2 = 128;
    
    while (rainbowOn) {
        // Update LED 0 with its current hue
        strip.setPixelColor(0, Wheel(hue1, strip));
        // Update LED 1 with its current hue
        strip.setPixelColor(1, Wheel(hue2, strip));
        strip.show();
        
        // Increment the hue values for a smooth color transition
        hue1++;
        hue2++;
        
        // Process any queued events so nothing backs up
        queHandler.Que();
        
        // Delay for a short time to control the transition speed (adjust as needed)
        delay(20);
    }
}

/**
 * @brief Turns off the rainbow effect.
 *
 * This function stops the rainbow cycle loop and turns off the LEDs.
 */
void RGBHandler::offRainbow() {
    rainbowOn = false;  // This will break the loop in rainbowCycle()
    off();            // Optionally, call your off() function to clear the LEDs.
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
