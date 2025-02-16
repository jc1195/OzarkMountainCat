#include "bleHandler.h"

// Constructor - nothing special needed here
BleHandler::BleHandler() {}

void connect_callback(uint16_t conn_handle) {
    Serial.println("BLE connected!");
  }
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    Serial.println("BLE disconnected!");
    Serial.println(reason);
}

// Initialize BLE
void BleHandler::begin()
{
    
    // Initialize the Bluefruit stack
    Bluefruit.begin();
    Bluefruit.Periph.begin();
    //Bluefruit.configPrphConn(247);
    Bluefruit.Periph.setConnInterval(16, 80);
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
    // Set the device name so your web app can filter by "MountainCat"
    Bluefruit.setName("MountainCat");
    
    Bluefruit.autoConnLed(false);
    Bluefruit.setTxPower(8); // Adjust if your signal is weak -40, -20, -16, -12, -8, -4, 0, 2, 3, 4, 5, 6, 7, 8 
    
    //mountainCatChar.

    // 1) Create the BLE service
    bledis.setModel("RAK4630");
    bledis.setManufacturer("RAKwireless");
    bledis.begin();

    mountainCatService.begin();
    

    // 2) Configure the characteristic
    //    - READ: phone can read the current value
    //    - WRITE: phone can send commands
    //    - NOTIFY: device can push updates to the phone
    mountainCatChar.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE |CHR_PROPS_NOTIFY);
    // Set the max length for your data. Adjust if your LoRa packets are large
    mountainCatChar.setMaxLen(247); // often 247 is safe if MTU extended
    // If the phone writes data, it calls onWriteCallback
    //mountainCatChar.indicate();
    //mountainCatChar.setUuid(MOUNTAINCAT_CHARACTERISTIC_UUID);
    mountainCatChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    mountainCatChar.setWriteCallback(BleHandler::onWriteCallback);
    //mountainCatChar.
    //mountainCatChar.setNotifyCallback(NULL);
    //Bluefruit.Central.
    mountainCatChar.setUuid(MOUNTAINCAT_CHARACTERISTIC_UUID);
    mountainCatChar.begin();
    
    // 3) Set up advertising
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    // Add our service's UUID to the advertisement so the phone knows we have it
    Bluefruit.Advertising.addService(mountainCatService);
    // Include the device name in the advertising packet
    Bluefruit.Advertising.addName();
    
    // Start advertising
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(160, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);	 // number of seconds in fast mode
    Bluefruit.Advertising.start(0);				 // 0 = Don't stop advertising after n seconds
        
    Serial.println("BLE initialized. Advertising as 'MountainCat'...");
}

void BleHandler::update()
{
    // Typically, Adafruit/Bluefruit BLE processes in the background automatically.
    // If you had other BLE housekeeping to do, you'd put it here.
}

void BleHandler::sendData(const uint8_t* data, uint16_t length)
{
    if (!isConnected()) {
        Serial.println("BLE not connected, skipping sendData.");
        return;
    }
    // // Debug print
    // Serial.print("Sent BLE data: ");
    // // Print as ASCII for debug. If your data is binary, this might look weird
    // for (int i = 0; i < length; i++) {
    //     Serial.write(data[i]);
    // }

    // If your data is larger than the characteristic can handle at once, you might need to split it.
    if (length > mountainCatChar.getMaxLen()) {
        length = mountainCatChar.getMaxLen();
        Serial.println("Warning: Data truncated to characteristic max length.");
    }

    // Write the value to the characteristic
    // mountainCatChar.setValue(data, length);
    // mountainCatChar.notify();
    // Notify the phone so it receives 'characteristicvaluechanged' event
    mountainCatChar.notify(data, length);
    Serial.print("Sent BLE data: ");
    Serial.println();
}

// Check if at least one device is connected
bool BleHandler::isConnected()
{
    // With the Adafruit nRF52 stack, you can check connection count
    // If you have multiple connections, you could refine logic
    return (Bluefruit.connected() > 0);
}

// Static callback if the phone writes to our characteristic
void BleHandler::onWriteCallback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
    Serial.print("BLE Write from conn_handle ");
    Serial.println(conn_handle);

    Serial.print("Data length: ");
    Serial.println(len);

    Serial.print("Data: ");
    for (int i = 0; i < len; i++) {
        Serial.write(data[i]);
    }
    Serial.println();

    // If you want to interpret these as ASCII commands, you could do so:
    // e.g., "RED", "LIVE", "POWER_SAVE", etc.
    // ...
}
