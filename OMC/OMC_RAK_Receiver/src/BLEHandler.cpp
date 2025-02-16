#include "BLEHandler.h"

// Constructor - nothing special needed here
BleHandler::BleHandler() {}
bool bleReceived = false;

void connect_callback(uint16_t conn_handle) {
    Serial.println("BLE connected!");
     // Get the reference to current connection
     // The below code is necessary to get past the 20 byte MTU limit. This is the only way the code will work. 
     // Metods like "Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);"" don't work unless you have the below code.
    BLEConnection* connection = Bluefruit.Connection(conn_handle);

    char central_name[32] = { 0 };
    connection->getPeerName(central_name, sizeof(central_name));

    Serial.println("Request to change MTU size to 200 bytes");
    connection->requestMtuExchange(203);                              // Change MTU SIZE

    Serial.print("Connected to ");
    Serial.println(central_name);

    // delay a bit for all the request to complete
    delay(1000);
  }
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    Serial.println("BLE disconnected!");
    Serial.println(reason);
}

// Initialize BLE
void BleHandler::begin()
{
    // Initialize the Bluefruit stack
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
    Bluefruit.begin();
    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
    // Set the device name so your web app can filter by "MountainCat"
    Bluefruit.setName("MountainCat");
    //Bluefruit.autoConnLed(false);
    Bluefruit.setTxPower(8); // Adjust if your signal is weak -40, -20, -16, -12, -8, -4, 0, 2, 3, 4, 5, 6, 7, 8 
    
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
    // If the phone writes data, it calls onWriteCallback
    mountainCatChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    mountainCatChar.setWriteCallback(BleHandler::onWriteCallback);
    mountainCatChar.setUuid(MOUNTAINCAT_CHARACTERISTIC_UUID);
    mountainCatChar.setFixedLen(200);
    mountainCatChar.begin();
    //mountainCatChar.setMaxLen(247);

    // 3) Set up advertising
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    // Add our service's UUID to the advertisement so the phone knows we have it
    Bluefruit.Advertising.addService(mountainCatService);
    // Include the device name in the advertising packet
    Bluefruit.Advertising.addName();
    
    // Start advertising
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 64); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);	 // number of seconds in fast mode
    Bluefruit.Advertising.start(0);				 // 0 = Don't stop advertising after n seconds
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
    Bluefruit.configPrphConn(247, 200, 4, 4);    
    Serial.println("BLE initialized. Advertising as 'MountainCat'...");
}

void BleHandler::update()
{
    //This is a placeholder for the future
}

void BleHandler::sendData(const uint8_t* data, uint16_t length)
{
    if (!isConnected()) {
        Serial.println("BLE not connected, skipping sendData.");
        return;
    }
    if (length > mountainCatChar.getMaxLen()) {
        length = mountainCatChar.getMaxLen();
        Serial.println("Warning: Data truncated to characteristic max length.");
    }
    mountainCatChar.notify(data, length);
    Serial.print("Sent BLE data: ");
}

// Check if at least one device is connected
bool BleHandler::isConnected()
{
    return (Bluefruit.connected() > 0);
}

void BleHandler::OnRxToJSON(uint8_t *payload, uint16_t len)
{
    // Now parse with ArduinoJson
    char jsonBuffer[len + 1];
    memcpy(jsonBuffer, payload, len);
    jsonBuffer[len] = '\0';  // Null terminate the string

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, jsonBuffer);
    if (!error)
    {
        // Extract fields
        receivedPacket.msgType = doc["msgType"];
        delay(10);
        if (receivedPacket.msgType == MSG_LED)
        {
            receivedPacket.r = doc["r"];
            receivedPacket.g = doc["g"];
            receivedPacket.b = doc["b"];
        }
        if (receivedPacket.msgType == MSG_RB_LED)
        {
            receivedPacket.rbLed = doc["rbLed"]; // rainbow led
        }
        if (receivedPacket.msgType == MSG_PWR_MODE)
        {
            receivedPacket.mode = doc["mode"];
        }
        if (receivedPacket.msgType == MSG_BUZZER)
        {
            receivedPacket.buzzer = doc["buzzer"];
        }
        if (receivedPacket.msgType == MSG_ACKNOWLEDGEMENT)
        {
            receivedPacket.ack = doc["ack"];
        }
        if (receivedPacket.msgType == MSG_ALL_DATA)
        {
            receivedPacket.lat = doc["lat"];
            receivedPacket.lon = doc["lon"];
            receivedPacket.hour = doc["hour"];
            receivedPacket.min = doc["min"];
            receivedPacket.sec = doc["sec"];
            receivedPacket.siv = doc["siv"];
            receivedPacket.hdop = doc["hdop"];
            receivedPacket.alt = doc["alt"];
        };
    }
    else
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
    }
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
    OnRxToJSON(data, len);
    bleReceived = true;
}


void BleHandler::SerializeJSON(MessageType msgType, uint8_t* data)
{

    StaticJsonDocument<200> doc;
    if (msgType == MSG_ALL_DATA)
    {
        doc["lat"] = receivedPacket.lat;
        doc["lon"] = receivedPacket.lon;
        doc["hour"] = receivedPacket.hour;
        doc["min"] = receivedPacket.min;
        doc["sec"] = receivedPacket.sec;
        doc["siv"] = receivedPacket.siv;
        doc["hdop"] = receivedPacket.hdop;
        doc["alt"] = receivedPacket.alt;
    }
    else if (msgType == MSG_ACKNOWLEDGEMENT)
    {
        doc["ack"] = receivedPacket.ack;
    }
    else if (msgType == MSG_BUZZER)
    {
        doc["buzzer"] = receivedPacket.buzzer;
    }
    else if (msgType == MSG_LED)
    {
        doc["r"] = receivedPacket.r;
        doc["g"] = receivedPacket.g;
        doc["b"] = receivedPacket.b;
    }
    else if (msgType == MSG_RB_LED)
    {
        doc["rbLed"] = receivedPacket.rbLed; // rainbow led
    }
    else if (msgType == MSG_PWR_MODE)
    {
        doc["mode"] = receivedPacket.mode;
    }

    char buffer[200];
    size_t n = serializeJson(doc, buffer, sizeof(buffer));
    memset(data, 0, sizeof(data)); // Wipes entire buffer
    memcpy(data, buffer, n);            // Copy new payload
    data[n] = '\0';                     // Null-terminate the new data
}

