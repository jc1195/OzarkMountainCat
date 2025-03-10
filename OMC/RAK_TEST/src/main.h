#pragma once
/**
 * @file main.h
 * @brief Global definitions, macros, enums, structures, and external declarations 
 *        for the OzarkMountainCat project.
 *
 * This header file contains pin definitions, configuration macros, enumerations, 
 * the ReceivedPacket structure, and forward declarations for key classes used in the project.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <ArduinoJson.h>
//#include <SX126x-RAK4630.h> // RAK provided LoRa library
#include <queue.h>
#include "SX126x-Arduino.h"

//#include <Tone.h>
//#include <Adafruit_TinyUSB.h>
// #include <Wire.h>
// #include <SPI.h>

/**
 * @brief Pin Definitions
 * 
 * Adjust these as needed to match your hardware wiring.
 */
#define PIN_NEOPIXEL                9    /**< Pin used for NeoPixel LED. */
#define PIN_BUZZER                  WB_IO5  /**< Pin used for the buzzer. */

/**
 * @brief RF and LoRa configuration parameters.
 */
#define RF_FREQUENCY                915300000   /**< RF frequency in Hz (example for EU). */
#define TX_OUTPUT_POWER             22          /**< TX output power in dBm. */
#define LORA_BANDWIDTH              2           /**< LoRa Bandwidth (0:125kHz). */
#define LORA_SPREADING_FACTOR       11          /**< LoRa Spreading Factor. */
#define LORA_CODINGRATE             4           /**< LoRa Coding Rate (1 = 4/5). */
#define LORA_PREAMBLE_LENGTH        8           /**< LoRa preamble length. */
#define LORA_FIX_LENGTH_PAYLOAD_ON  false       /**< LoRa fixed-length payload flag. */
#define LORA_IQ_INVERSION_ON        false       /**< LoRa IQ inversion flag. */
#define TX_TIMEOUT_VALUE            3000        /**< TX timeout in milliseconds. */
#define RX_TIMEOUT_VALUE            0           /**< RX timeout in milliseconds. */
#define LORA_SYMBOL_TIMEOUT         0	        /**< LoRa symbol timeout (in symbols). */
#define LORA_DIO_PIN                47          /**< LoRa DIO pin number. */

/**
 * @brief Battery voltage measurement parameters.
 */
#define PIN_VBAT                    WB_A0       /**< Pin used for battery voltage measurement. */
#define VBAT_MV_PER_LSB             (0.73242188F) /**< Battery mV per LSB (ADC resolution calculation). */
#define VBAT_DIVIDER_COMP           (1.73)      /**< Compensation factor for the battery divider. */
#define REAL_VBAT_MV_PER_LSB        (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB) /**< Adjusted mV per LSB. */

/**
 * @brief Sleep durations for different power modes.
 * 
 * These macros define the sleep durations (in milliseconds) for each power mode.
 */
#define TIME_lIVE_TRACKING          ((uint32_t)15000)   /**< Live Tracking Mode: 15 seconds. */
#define TIME_POWER_SAVING           ((uint32_t)300000)  /**< Power Saving Mode: 5 minutes. */
#define TIME_EXTREME_POWER_SAVING   ((uint32_t)600000)  /**< Extreme Power Saving Mode: 10 minutes. */

/**
 * @brief External flag indicating if a packet was received.
 */
extern bool packetReceived;

/**
 * @brief Device operating modes.
 */
enum DeviceMode {
    MODE_LIVE_TRACKING = 0,       /**< Live Tracking Mode. */
    MODE_POWER_SAVING = 1,        /**< Power Saving Mode. */
    MODE_EXTREME_POWER_SAVING = 2 /**< Extreme Power Saving Mode. */
};

/**
 * @brief Message types used in communication.
 */
enum MessageType {
    MSG_ALL_DATA = 0,           /**< All Data Message. */
    MSG_ACKNOWLEDGEMENT = 1,    /**< Acknowledgement Message. */
    MSG_BUZZER = 2,             /**< Buzzer Message. */
    MSG_LED = 3,                /**< LED Message. */
    MSG_RB_LED = 4,           /**< Rainbow LED Message. */
    MSG_PWR_MODE = 5,           /**< Power Mode Message. */
    MSG_WAKE_TIMER = 6          /**< Wake Timer Message. */
};

/**
 * @brief Event types for the command queue.
 */
enum EventType {
    EVENT_LED = 0,            /**< LED event. */
    EVENT_ACKNOWLEDGEMENT = 1,/**< Acknowledgement event. */
    EVENT_BUZZER = 2,         /**< Buzzer event. */
    EVENT_RB_LED = 3,         /**< Rainbow LED event. */
    EVENT_PWR_MODE = 4,       /**< Power Mode change event. */
    EVENT_WAKE_TIMER = 5,     /**< Wake Timer event. */
    EVENT_LORA_RX = 6         /**< LoRa RX event. */
};

/**
 * @brief Structure representing a received packet.
 *
 * Contains various fields corresponding to sensor data and control flags.
 */
struct ReceivedPacket{
    MessageType msgType;   /**< Message type indicating the purpose of the packet. */
    double lat;            /**< Latitude. */
    double lon;            /**< Longitude. */
    DeviceMode mode;       /**< Operating mode of the device. */
    bool led;              /**< LED state flag. */
    bool rbLed;            /**< Rainbow LED state flag. */
    bool buzzer;           /**< Buzzer state flag. */
    uint8_t hour;          /**< Hour (time) value. */
    uint8_t min;           /**< Minute (time) value. */
    uint8_t sec;           /**< Second (time) value. */
    uint8_t siv;           /**< Satellites in view. */
    uint16_t hdop;         /**< HDOP value. */
    double alt;            /**< Altitude. */
    int16_t rssi;          /**< RSSI (signal strength). */
    int8_t snr;            /**< SNR value. */
    uint8_t r;              /**< Red channel value (for LED). */
    uint8_t g;              /**< Green channel value (for LED). */
    uint8_t b;              /**< Blue channel value (for LED). */
    bool ack;              /**< Acknowledgement flag. */
    uint8_t rBatt;           /**< Receiver battery level. */
    uint8_t hBatt;           /**< Harness battery level. */
};

/**
 * @brief Forward declaration of the BuzzerHandler class.
 */
class BuzzerHandler;

/**
 * @brief Forward declaration of the GPSHandler class.
 */
class GPSHandler;

/**
 * @brief Forward declaration of the LoraHandler class.
 */
class LoraHandler;

/**
 * @brief Forward declaration of the BleHandler class.
 */
class BleHandler;

/**
 * @brief Global semaphore for wake signals.
 *
 * This semaphore is used to wake the device when a LoRa event or timer event occurs.
 */
extern SemaphoreHandle_t wakeSemaphore;

/**
 * @brief Global command queue.
 *
 * This queue is used to store events that need to be processed by the system.
 */
extern QueueHandle_t commandQueue;

/**
 * @brief Global instance of the received packet.
 *
 * Contains all the data fields that are updated by incoming messages.
 */
extern ReceivedPacket receivedPacket;

/**
 * @brief Global instance of the current event type.
 */
extern EventType eventType;

//extern SoftwareTimer taskWakeupTimer;
