#pragma once
#include "main.h"

class RgbLed {
public:
    RgbLed(uint8_t pin, uint8_t num_pixels);
    void begin();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    bool isOn() { return ledOn; }

private:
    Adafruit_NeoPixel pixels;
    bool ledOn = false;
};
