#pragma once
/**
 * @file lora.h
 * @brief Header file for the LoraHandler class.
 *
 * This file contains the declarations for the LoraHandler class which manages
 * LoRa communications including initialization, sending JSON packets, handling
 * radio events, and processing received data.
 */

#include "main.h"

/**
 * @brief Global receive buffer for LoRa packets.
 *
 * The RcvBuffer is used to store the raw data of the most recently received LoRa packet.
 */
extern uint8_t RcvBuffer[200];

/**
 * @class LoraHandler
 * @brief Manages LoRa communications.
 *
 * The LoraHandler class encapsulates functions for initializing the LoRa radio,
 * sending JSON-formatted packets, handling radio callbacks (e.g., TX/RX done, errors),
 * and converting received data into a JSON structure.
 */
class LoraHandler {
public:
    /**
     * @brief Default constructor.
     */
    LoraHandler() {}

    /**
     * @brief Initializes the LoRa radio and its parameters.
     *
     * This function initializes the LoRa chip, sets up radio event callbacks,
     * configures the radio for TX and RX, and starts the radio in RX mode.
     */
    void begin();

    /**
     * @brief Updates the LoRa state.
     *
     * Currently a placeholder. If reception is enabled, this could be used to re-enter
     * RX mode or perform periodic maintenance.
     */
    void update();

    /**
     * @brief Sends a JSON packet containing all data (MSG_ALL_DATA) over LoRa.
     *
     * This function serializes GPS and sensor data into a JSON object and sends it via LoRa.
     *
     * @param lat Latitude.
     * @param lon Longitude.
     * @param hour Hour (time).
     * @param min Minute (time).
     * @param sec Second (time).
     * @param siv Satellites in view.
     * @param hdop HDOP value.
     * @param alt Altitude.
     */
    void SendJSON(double lat, double lon, 
                    uint8_t hour, uint8_t min, uint8_t sec, 
                    uint8_t siv, uint16_t hdop, double alt);

    /**
     * @brief Serializes JSON based on message type and stores it in the global receive buffer.
     *
     * This static function creates a JSON document using the current contents of the
     * global receivedPacket structure based on the provided message type, serializes it,
     * and copies it to RcvBuffer.
     *
     * @param msgType The message type to serialize.
     */
    static void SerializeJSON(MessageType msgType);

    /**
     * @brief Sends an acknowledgement JSON packet (MSG_ACKNOWLEDGEMENT) over LoRa.
     *
     * This function serializes an acknowledgement flag and additional default values into a JSON
     * document and sends it via LoRa.
     *
     * @param ack Boolean flag indicating acknowledgement status.
     */
    void SendJSON(bool ack);

    // The following functions are placeholders for other message types:
    // MSG_BUZZER, MSG_LED, MSG_RB_LED, MSG_PWR_MODE can be implemented as needed.

    /**
     * @brief Checks if a command has been received.
     *
     * @return Always returns false (placeholder implementation).
     */
    bool commandReceived() { return false; }

    /**
     * @brief Checks if the LED should be on.
     *
     * @return Always returns false (placeholder implementation).
     */
    bool shouldLightBeOn() { return false; }

    /**
     * @brief Checks if the buzzer should be activated.
     *
     * @return Always returns false (placeholder implementation).
     */
    bool shouldBuzzerBeOn() { return false; }

    /**
     * @brief Retrieves a pointer to the received packet buffer.
     *
     * @return Pointer to the global RcvBuffer containing the last received LoRa packet.
     */
    uint8_t* GetRxPacket(void);

private:
    /**
     * @brief Sends a packet over the LoRa radio.
     *
     * This private function is used by the SendJSON functions to transmit data.
     *
     * @param buffer Pointer to the data buffer to send.
     * @param size Size (in bytes) of the data to send.
     */
    void sendPacket(uint8_t *buffer, uint8_t size);

    // Static callback functions required by the SX126x driver:

    /**
     * @brief Callback called when a LoRa transmission is completed successfully.
     */
    static void OnTxDone(void);

    /**
     * @brief Callback called when a LoRa transmission times out.
     */
    static void OnTxTimeout(void);

    /**
     * @brief Callback called when a LoRa packet is received.
     *
     * @param payload Pointer to the received payload.
     * @param size Size (in bytes) of the received payload.
     * @param rssi Received Signal Strength Indicator.
     * @param snr Signal-to-Noise Ratio.
     */
    static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

    /**
     * @brief Callback called when LoRa reception times out.
     */
    static void OnRxTimeout(void);

    /**
     * @brief Callback called when a LoRa reception error occurs.
     */
    static void OnRxError(void);

    /**
     * @brief Placeholder function for receiving data.
     */
    static void Recieve(void);

    /**
     * @brief Reads the IRQ status from the radio.
     *
     * @return uint16_t The combined IRQ status from the radio.
     */
    static uint16_t readIrqStatus(void);

    /**
     * @brief Clears specific IRQ status flags on the radio.
     *
     * @param irqStatus The IRQ status flags to clear.
     */
    static void clearIrqStatus(uint16_t irqStatus);

    /**
     * @brief Converts a received payload to JSON and updates the global receivedPacket.
     *
     * This function parses the payload using ArduinoJson and extracts fields based on the message type.
     *
     * @param payload Pointer to the received payload.
     * @param rssi Received Signal Strength Indicator.
     * @param snr Signal-to-Noise Ratio.
     */
    static void OnRxToJSON(uint8_t *payload, int16_t rssi, int8_t snr);

    /**
     * @brief Queues an event based on the current message type in receivedPacket.
     *
     * This static function examines receivedPacket.msgType and sends corresponding events into the commandQueue.
     */
    static void queEvent();

    /**
     * @brief Pointer to the singleton instance of LoraHandler.
     *
     * This static variable is used to associate static callback functions with the current instance.
     */
    static LoraHandler* instance;

    /**
     * @brief Flag indicating whether the LoRa radio has been successfully initialized.
     */
    bool loraInitialized = false;
};
