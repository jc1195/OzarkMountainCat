#pragma once
#include "main.h"

class BuzzerHandler {
public:
    BuzzerHandler(uint8_t pin): buzzerPin(pin) {}
    void begin() {
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW);
        buzzerOn = false;
    }
    void on() {
        digitalWrite(buzzerPin, HIGH);
        buzzerOn = true;
    }
    void off() {
        digitalWrite(buzzerPin, LOW);
        buzzerOn = false;
    }
    bool isOn() { return buzzerOn; }

private:
    uint8_t buzzerPin;
    bool buzzerOn = false;
};
