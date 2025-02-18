#pragma once

#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <Adafruit_NeoPixel.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <ArduinoJson.h>
#include <SX126x-RAK4630.h> // RAK provided LoRa library
#include <queue.h>


// Pin Definitions (Adjust as needed)
#define NEOPIXEL_PIN                9    // Example pin. Make sure this matches your wiring
#define BUZZER_PIN                  10

#define RF_FREQUENCY                915300000   // US Frequency
#define TX_OUTPUT_POWER             22
#define LORA_BANDWIDTH              2           // 0:125kHz
#define LORA_SPREADING_FACTOR       11
#define LORA_CODINGRATE             4           // 1=4/5
#define LORA_PREAMBLE_LENGTH        8
#define LORA_FIX_LENGTH_PAYLOAD_ON  false
#define LORA_IQ_INVERSION_ON        false
#define TX_TIMEOUT_VALUE            3000
#define RX_TIMEOUT_VALUE            0
#define LORA_SYMBOL_TIMEOUT         0	// Symbols

// Battery Definitions
#define PIN_VBAT                    WB_A0
#define VBAT_MV_PER_LSB             (0.73242188F) // 3.0V ADC range and 12 - bit ADC resolution = 3000mV / 4096
#define VBAT_DIVIDER_COMP           (1.73)      // Compensation factor for the VBAT divider, depend on the board  
#define REAL_VBAT_MV_PER_LSB        (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

//flags
extern bool packetReceived;
extern bool bleReceived;

// If using I2C for GNSS, RAK4631 defaults: SDA & SCL are on Wire
// Mode enumerations
enum DeviceMode {
    MODE_LIVE_TRACKING = 0,
    MODE_POWER_SAVING = 1,
    MODE_EXTREME_POWER_SAVING = 2
};

enum MessageType {
    MSG_ALL_DATA = 0,
    MSG_ACKNOWLEDGEMENT = 1,
    MSG_BUZZER = 2,
    MSG_LED = 3,
    MSG_RB_LED = 4, // rainbow led
    MSG_PWR_MODE = 5
};

enum EventType {
    EVENT_LED = 0,
    EVENT_ACKNOWLEDGEMENT = 1,
    EVENT_BUZZER = 2,
    EVENT_RB_LED = 3, // rainbow led
    EVENT_PWR_MODE = 4,
    EVENT_WAKE_TIMER = 5,
    EVENT_LORA_RX = 6
};

struct ReceivedPacket{
    MessageType msgType;
    double lat;
    double lon;
    DeviceMode mode;
    bool led;
    bool rbLed; // rainbow led
    bool buzzer;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t siv;
    uint16_t hdop;
    double alt;
    int16_t rssi;
    int8_t snr;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    bool ack;
    float rBatt; //Receiver battery
    float hBatt; //Harness battery
    
};

// Declare a global instance of the struct
extern ReceivedPacket receivedPacket;


// Forward declarations
class BleHandler;
class LoraHandler;
class RgbLed;
class GnssHandler;
class BuzzerHandler;
class PowerManager;

// Global semaphore for wake signals
//extern SemaphoreHandle_t wakeSemaphore;
