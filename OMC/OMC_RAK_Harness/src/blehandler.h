#pragma once

#include "main.h"
#include <bluefruit.h>
// #include <wire.h>

// You can define your 128-bit UUIDs as strings:
#define MOUNTAINCAT_SERVICE_UUID       "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define MOUNTAINCAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// #define MOUNTAINCAT_SERVICE_UUID       "0x1234"
//#define MOUNTAINCAT_CHARACTERISTIC_UUID "0x4231"

// This class sets up a BLE service with the "MountainCat" name and a single characteristic.
// It supports read, write, and notify so the phone can receive data (LoRa packets) and send commands.
class BleHandler {
public:
    BleHandler();
    void begin();
    void update();

    // Call this to send data to the phone. 'data' is your buffer, 'length' is how many bytes.
    void sendData(const uint8_t* data, uint16_t length);
    // Callback when the phone writes to our characteristic
    static void onWriteCallback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len);

private:
    // A helper to see if at least one device is connected over BLE
    bool isConnected();
    BLEDis bledis; // DIS (Device Information Service) helper class instance
    BLEBas blebas; // BAS (Battery Service) helper class instance

    // Our BLE service and characteristic
    BLEService        mountainCatService = BLEService(MOUNTAINCAT_SERVICE_UUID);
    BLECharacteristic mountainCatChar    = BLECharacteristic(MOUNTAINCAT_CHARACTERISTIC_UUID);
    
    
};
