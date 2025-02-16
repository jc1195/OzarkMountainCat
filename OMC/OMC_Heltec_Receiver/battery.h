#include <Arduino.h>
#include <esp_adc_cal.h>
#include <driver/adc.h>
#include "heltec.h"

class Battery{
    public:

        //This function needs to be called in the setup function of
        //your program.
        //"withDisplay" means you want to use the "drawBattery" function
        //and have it appear on the display.
        void begin(bool withDisplay);

        //This function reads the batterys voltage while providing a accurate averaged out reading.
        uint16_t GetVBatt();

        //This function will draw a battery icon in the corner of the screen.
        //Voltage: The voltage from the battery.
        //Sleep: Give a true or false value if you want the "z" to be drawn in the battery icon when esp32 goes to sleep.
        void drawBattery(uint16_t voltage, bool sleep);
};
extern Battery Batt;