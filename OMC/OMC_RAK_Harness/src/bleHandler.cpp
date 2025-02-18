#include "bleHandler.h"

/**
 * @brief Constructor for the BleHandler class.
 *
 * This is a default constructor that currently does not require any special initialization.
 */
BleHandler::BleHandler() {}

/**
 * @brief Callback function called when a BLE connection is established.
 *
 * This function is invoked when a device connects over BLE.
 *
 * @param conn_handle The connection handle of the established connection.
 */
void connect_callback(uint16_t conn_handle) {
    //Serial.println("BLE connected!");
}

/**
 * @brief Callback function called when a BLE connection is disconnected.
 *
 * This function is invoked when a BLE connection is terminated.
 *
 * @param conn_handle The connection handle of the disconnected connection.
 * @param reason The reason code for the disconnection.
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    //Serial.println("BLE disconnected!");
    //Serial.println(reason);
}

/**
 * @brief Initializes BLE functionality.
 *
 * This function initializes the Bluefruit stack and configures the device as a BLE Peripheral.
 * It sets connection parameters, device name, TX power, and creates a BLE service with a characteristic.
 * It also configures advertising parameters so that the device advertises as "MountainCat".
 */
void BleHandler::begin()
{
    // Initialize the Bluefruit BLE stack.
    Bluefruit.begin();
    Bluefruit.Periph.begin();
    // Bluefruit.configPrphConn(247); // Optionally configure connection settings.
    Bluefruit.Periph.setConnInterval(16, 80);
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
    
    // Set the device name so that the web app can filter by "MountainCat".
    Bluefruit.setName("MountainCat");
    
    // Disable automatic connection LED.
    Bluefruit.autoConnLed(false);
    
    // Set TX power (adjust based on your signal strength needs).
    Bluefruit.setTxPower(8); // Options: -40, -20, -16, -12, -8, -4, 0, 2, 3, 4, 5, 6, 7, 8 
    
    // Configure Device Information Service.
    bledis.setModel("RAK4630");
    bledis.setManufacturer("RAKwireless");
    bledis.begin();

    // Begin the custom service for the MountainCat application.
    mountainCatService.begin();
    
    // 2) Configure the characteristic:
    //    Set properties: READ (phone can read current value), WRITE (phone can send commands),
    //    NOTIFY (device can push updates to the phone).
    mountainCatChar.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE | CHR_PROPS_NOTIFY);
    // Set maximum data length for the characteristic (e.g., 247 bytes).
    mountainCatChar.setMaxLen(247); // Often 247 is safe if MTU extended.
    
    // If the phone writes data, it calls onWriteCallback.
    mountainCatChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    mountainCatChar.setWriteCallback(BleHandler::onWriteCallback);
    mountainCatChar.setUuid(MOUNTAINCAT_CHARACTERISTIC_UUID);
    mountainCatChar.begin();
    
    // 3) Set up advertising:
    //    Add flags for general discoverability, include TX power, service UUID, and device name.
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(mountainCatService);
    Bluefruit.Advertising.addName();
    
    // Configure advertising to restart on disconnect and set intervals.
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(160, 244); // Intervals in units of 0.625 ms.
    Bluefruit.Advertising.setFastTimeout(30);     // Fast advertising mode duration (seconds).
    Bluefruit.Advertising.start(0);               // 0 = Do not stop advertising after n seconds.
        
    //Serial.println("BLE initialized. Advertising as 'MountainCat'...");
}

/**
 * @brief Updates BLE-related tasks.
 *
 * This function is intended for periodic BLE housekeeping. For the Adafruit/Bluefruit BLE stack,
 * background processing occurs automatically, so no additional code is required here.
 */
void BleHandler::update()
{
    // Typically, Bluefruit BLE processes in the background.
    // If additional BLE housekeeping is needed, implement it here.
}

/**
 * @brief Sends data over BLE.
 *
 * This function sends the given data (as a byte array) with the specified length over BLE.
 * If the data size exceeds the maximum characteristic length, it is truncated.
 *
 * @param data Pointer to the data to send.
 * @param length Length of the data in bytes.
 */
void BleHandler::sendData(const uint8_t* data, uint16_t length)
{
    if (!isConnected()) {
        //Serial.println("BLE not connected, skipping sendData.");
        return;
    }
    // Debug print (commented out):
    // //Serial.print("Sent BLE data: ");
    // for (int i = 0; i < length; i++) {
    //     //Serial.write(data[i]);
    // }

    // If the data exceeds the max length of the characteristic, truncate it.
    if (length > mountainCatChar.getMaxLen()) {
        length = mountainCatChar.getMaxLen();
        //Serial.println("Warning: Data truncated to characteristic max length.");
    }

    // Notify the phone with the data so that it receives a 'characteristicvaluechanged' event.
    mountainCatChar.notify(data, length);
    //Serial.print("Sent BLE data: ");
    //Serial.println();
}

/**
 * @brief Checks if a BLE device is connected.
 *
 * This function returns true if at least one BLE connection is active.
 *
 * @return true if connected, false otherwise.
 */
bool BleHandler::isConnected()
{
    // With the Adafruit nRF52 stack, use Bluefruit.connected() to get connection count.
    return (Bluefruit.connected() > 0);
}

/**
 * @brief Static callback function invoked when the BLE characteristic is written to.
 *
 * This function is called when the phone writes to the characteristic. It prints connection
 * and data information for debugging purposes.
 *
 * @param conn_handle The connection handle of the writing device.
 * @param chr Pointer to the BLECharacteristic that was written to.
 * @param data Pointer to the data received.
 * @param len Length of the data received.
 */
void BleHandler::onWriteCallback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
    //Serial.print("BLE Write from conn_handle ");
    //Serial.println(conn_handle);

    //Serial.print("Data length: ");
    //Serial.println(len);

    //Serial.print("Data: ");
    for (int i = 0; i < len; i++) {
        //Serial.write(data[i]);
    }
    //Serial.println();

    // Here you can add code to interpret the incoming data as ASCII commands,
    // e.g., "RED", "LIVE", "POWER_SAVE", etc.
}
