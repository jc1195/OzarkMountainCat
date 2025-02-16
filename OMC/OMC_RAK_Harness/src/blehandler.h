#pragma once
/**
 * @file bleHandler.h
 * @brief Header file for the BleHandler class.
 *
 * This file declares the BleHandler class which handles BLE communications.
 * It sets up a BLE service with the "MountainCat" name and a single characteristic,
 * supporting read, write, and notify operations so that the phone can both send commands
 * and receive data (such as LoRa packets).
 */

#include "main.h"
#include <bluefruit.h>
// #include <wire.h>

/**
 * @name UUID Definitions
 * 
 * The following macros define the 128-bit UUIDs for the BLE service and characteristic.
 * You can define these as strings.
 * @{
 */
#define MOUNTAINCAT_SERVICE_UUID       "4fafc201-1fb5-459e-8fcc-c5c9c331914b"    /**< UUID for the MountainCat BLE service. */
#define MOUNTAINCAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"    /**< UUID for the MountainCat BLE characteristic. */
//#define MOUNTAINCAT_SERVICE_UUID       "0x1234"  // Alternate 16-bit UUID example.
//#define MOUNTAINCAT_CHARACTERISTIC_UUID "0x4231" // Alternate 16-bit UUID example.
/** @} */

/**
 * @class BleHandler
 * @brief Manages BLE communications for the MountainCat project.
 *
 * The BleHandler class initializes the Bluefruit BLE stack, sets up a BLE service and a characteristic,
 * and provides methods to send data to the phone as well as handle write callbacks.
 */
class BleHandler {
public:
    /**
     * @brief Constructor for BleHandler.
     *
     * Initializes the BleHandler object.
     */
    BleHandler();

    /**
     * @brief Initializes BLE functionality.
     *
     * Sets up the Bluefruit BLE stack, configures connection parameters, device name, TX power,
     * and the BLE service and characteristic. It also sets up advertising so that the device is discoverable.
     */
    void begin();

    /**
     * @brief Performs periodic BLE housekeeping tasks.
     *
     * Typically, the Bluefruit BLE stack handles background processing automatically.
     * Use this method if additional BLE maintenance tasks are needed.
     */
    void update();

    /**
     * @brief Sends data over BLE.
     *
     * Sends a byte buffer to the connected BLE device via the MountainCat characteristic.
     *
     * @param data Pointer to the data buffer to send.
     * @param length The number of bytes in the data buffer.
     */
    void sendData(const uint8_t* data, uint16_t length);

    /**
     * @brief Callback function invoked when the phone writes to the characteristic.
     *
     * This static method is called when data is written from the phone.
     * It prints debugging information including the connection handle and data length.
     *
     * @param conn_handle The connection handle of the phone.
     * @param chr Pointer to the BLECharacteristic that received the data.
     * @param data Pointer to the received data buffer.
     * @param len Length of the received data in bytes.
     */
    static void onWriteCallback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len);

private:
    /**
     * @brief Checks if at least one BLE device is connected.
     *
     * @return true if one or more devices are connected; false otherwise.
     */
    bool isConnected();

    /**
     * @brief Device Information Service helper.
     *
     * Provides information such as model and manufacturer details.
     */
    BLEDis bledis;

    /**
     * @brief Battery Service helper.
     *
     * Used for monitoring battery status over BLE.
     */
    BLEBas blebas;

    /**
     * @brief BLE service for the MountainCat application.
     *
     * Created with the defined MOUNTAINCAT_SERVICE_UUID.
     */
    BLEService        mountainCatService = BLEService(MOUNTAINCAT_SERVICE_UUID);

    /**
     * @brief BLE characteristic for the MountainCat application.
     *
     * Created with the defined MOUNTAINCAT_CHARACTERISTIC_UUID. It supports read, write, and notify operations.
     */
    BLECharacteristic mountainCatChar    = BLECharacteristic(MOUNTAINCAT_CHARACTERISTIC_UUID);
};
