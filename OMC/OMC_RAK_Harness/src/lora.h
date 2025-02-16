#pragma once

#include "main.h"

extern uint8_t RcvBuffer[200]; // Declare it as extern

class LoraHandler {
public:
    LoraHandler() {}
    void begin();
    void update();

    // MSG_ALL_DATA = 0
    void SendJSON(double lat, double lon, 
                    uint8_t hour, uint8_t min, uint8_t sec, 
                    uint8_t siv, uint16_t hdop, double alt);

    static void SerializeJSON(MessageType msgType);
    // MSG_ACKNOWLEDGEMENT = 1
    void SendJSON(bool ack);
    // MSG_BUZZER = 2
    //void SendJSON(MessageType msgType, bool buzzerStatus, int8_t r, int8_t g, int8_t b);
    // MSG_LED = 3
    //void SendJSON(MessageType msgType, int8_t r, int8_t g, int8_t b);
    // MSG_RB_LED = 4 // rainbow led
    // MSG_PWR_MODE = 5
    //void SendJSON(MessageType msgType, DeviceMode mode);

    //void processCommand(const char* command);

    bool commandReceived() { return false; }
    bool shouldLightBeOn() { return false; }
    bool shouldBuzzerBeOn() { return false; }
    //void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
    uint8_t* GetRxPacket();

private:
    void sendPacket(uint8_t *buffer, uint8_t size);

    // Static callbacks required by SX126x driver
    static void OnTxDone(void);
    static void OnTxTimeout(void);
    static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
    static void OnRxTimeout(void);
    static void OnRxError(void);
    static void Recieve(void);
    static uint16_t readIrqStatus(void);
    static void clearIrqStatus(uint16_t irqStatus);
    static void OnRxToJSON(uint8_t *payload, int16_t rssi, int8_t snr);

    // Associate callbacks with this instance
    static LoraHandler* instance;
    static void queEvent();

    bool loraInitialized = false;
};
