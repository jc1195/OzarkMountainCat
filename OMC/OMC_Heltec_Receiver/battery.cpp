#include <battery.h>

#define MAXBATT 4200             // The default Lipo is 4200mv when the battery is fully charged.
#define LIGHT_SLEEP_VOLTAGE 3750 // Point where start light sleep
#define MINBATT 3150             // The default Lipo is 3200mv when the battery is empty...this WILL be low on the 3.3v rail specs!!!

#define VOLTAGE_DIVIDER 3.20                     // Lora has 220k/100k voltage divider so need to reverse that reduction via (220k+100k)/100k on vbat GPIO37 or ADC1_1 (early revs were GPIO13 or ADC2_4 but do NOT use with WiFi.begin())
#define DEFAULT_VREF 1100                        // Default VREF use if no e-fuse calibration
#define VBATT_SAMPLE 500                         // Battery sample rate in ms
#define VBATT_SMOOTH 50                          // Number of averages in sample
#define ADC_READ_STABILIZE 5                     // in ms (delay from GPIO control and ADC connections times)
#define LO_BATT_SLEEP_TIME 10 * 60 * 1000 * 1000 // How long when low batt to stay in sleep (us)
#define HELTEC_V2_1 1                            // Set this to switch between GPIO13(V2.0) and GPIO37(V2.1) for VBatt ADC.
#define VBATT_GPIO 21                            // Heltec GPIO to toggle VBatt read connection ... WARNING!!! This also connects VEXT to VCC=3.3v so be careful what is on header.  Also, take care NOT to have ADC read connection in OPEN DRAIN when GPIO goes HIGH
#define __DEBUG 0                                // DEBUG Serial output

uint16_t Sample();
void drawBattery(uint16_t, bool = false);

esp_adc_cal_characteristics_t *adc_chars;

void Battery::begin(bool withDisplay)
{
    while (!Serial)
        ;
    delay(20);

    // Characterize ADC at particular atten
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_6, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_6);

    if (withDisplay)
    {
        Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
        Heltec.display->flipScreenVertically();
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->clear();
    }

    // Prime the Sample register
    for (uint8_t i = 0; i < VBATT_SMOOTH; i++)
    {
        Sample();
    }

    pinMode(VBATT_GPIO, OUTPUT);
    digitalWrite(VBATT_GPIO, LOW); // ESP32 Lora v2.1 reads on GPIO37 when GPIO21 is low
    delay(ADC_READ_STABILIZE);     // let GPIO stabilize
}

// void loop()
// {
//     // Heltec.display->clear();
//     uint16_t voltage = Sample();
//     // drawBattery(voltage, voltage < LIGHT_SLEEP_VOLTAGE);
//     //  Heltec.display->display();
//     // if (voltage < MINBATT)
//     // { // Low Voltage cut off shut down to protect battery as long as possible
//     //     Heltec.display->setColor(WHITE);
//     //     Heltec.display->setFont(ArialMT_Plain_10);
//     //     Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
//     //     Heltec.display->drawString(64, 24, "Shutdown!!");
//     //     Heltec.display->display();
//     //     delay(2000);
//     //     esp_sleep_enable_timer_wakeup(LO_BATT_SLEEP_TIME);
//     //     esp_deep_sleep_start();
//     // }
//     // else if (voltage < LIGHT_SLEEP_VOLTAGE)
//     // { // Use light sleep once on battery
//     //     uint64_t s = VBATT_SAMPLE;
//     //     esp_sleep_enable_timer_wakeup(s * 1000); // Light Sleep does not flush buffer
//     //     esp_light_sleep_start();
//     // }
//     delay(ADC_READ_STABILIZE);
// }

// Poll the proper ADC for VBatt on Heltec Lora 32 with GPIO21 toggled
uint16_t ReadVBatt()
{
    uint16_t reading = 666;

    digitalWrite(VBATT_GPIO, LOW); // ESP32 Lora v2.1 reads on GPIO37 when GPIO21 is low
    delay(ADC_READ_STABILIZE);     // let GPIO stabilize

    pinMode(ADC1_CHANNEL_1, OPEN_DRAIN); // ADC GPIO37
    reading = adc1_get_raw(ADC1_CHANNEL_1);
    pinMode(ADC1_CHANNEL_1, INPUT); // Disconnect ADC before GPIO goes back high so we protect ADC from direct connect to VBATT (i.e. no divider)

    uint16_t voltage = esp_adc_cal_raw_to_voltage(reading, adc_chars);
    voltage *= VOLTAGE_DIVIDER;

    return voltage;
}

//  Use a buffer to average/sample ADC
uint16_t Sample()
{
    static uint8_t i = 0;
    static uint16_t samp[VBATT_SMOOTH];
    static int32_t t = 0;
    static bool f = true;
    if (f)
    {
        for (uint8_t c = 0; c < VBATT_SMOOTH; c++)
        {
            samp[c] = 0;
        }
        f = false;
    }             // Initialize the sample array first time
    t -= samp[i]; // doing a rolling recording, so remove the old rolled around value out of total and get ready to put new one in.
    if (t < 0)
    {
        t = 0;
    }

    // ADC read
    uint16_t voltage = ReadVBatt();

    samp[i] = voltage;

    t += samp[i];

    if (++i >= VBATT_SMOOTH)
    {
        i = 0;
    }
    uint16_t s = round(((float)t / (float)VBATT_SMOOTH));

    return s;
}

void Battery::drawBattery(uint16_t voltage, bool sleep)
{
    Heltec.display->setColor(BLACK);
    Heltec.display->fillRect(99, 4, 29, 24);

    Heltec.display->setColor(WHITE);
    Heltec.display->drawRect(104, 4, 12, 6);
    Heltec.display->fillRect(116, 6, 1, 2);

    double pct = voltage;
    uint8_t bars = round(pct / 10.0);
    Heltec.display->fillRect(105, 5, bars, 4);
    //Heltec.display->setFont(ArialMT_Plain_10);
    //Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
    // Draw small "z" when using sleep
    if (sleep)
    {
        Heltec.display->drawHorizontalLine(121, 0, 4);
        Heltec.display->drawHorizontalLine(121, 5, 4);
        Heltec.display->setPixel(124, 1);
        Heltec.display->setPixel(123, 2);
        Heltec.display->setPixel(122, 3);
        Heltec.display->setPixel(121, 4);
    }
    // Heltec.display->drawString(127, 5, String((int)round(pct)) + "%");
    // Heltec.display->drawString(127, 14, String(round(voltage / 10.0) / 100.0) + "V");
}

uint16_t Battery::GetVBatt()
{
    uint16_t v = 0;
    for (int i = 0; i < 75; i++)
    {
        v = Sample();
    }
    //uint16_t v = Sample();
    if (v < MINBATT)
    {
        v = MINBATT;
    }
    if (v > MAXBATT)
    {
        v = MAXBATT;
    }
    double pct = map(v, MINBATT, MAXBATT, 0, 100);
    return pct;
}

Battery Batt;
