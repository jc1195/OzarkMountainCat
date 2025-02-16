#include "LoraHandler.h"

// extern QueueHandle_t eventQueue;
// extern SemaphoreHandle_t wakeSemaphore;

uint8_t RcvBuffer[200]; // Define the actual buffer
bool packetReceived = false;
ReceivedPacket receivedPacket = {};
LoraHandler *LoraHandler::instance = nullptr;
static RadioEvents_t RadioEvents;

#define SX126X_GET_IRQ_STATUS 0x15
#define SX126X_CLR_IRQ_STATUS 0x02

uint16_t LoraHandler::readIrqStatus()
{
    uint8_t buffer[2] = {0};
    // readCommand(opcode, destinationBuffer, numBytesToRead)
    // Make sure your Radio object is accessible here
    Radio.ReadBuffer(SX126X_GET_IRQ_STATUS, buffer, 2);

    // buffer[0] = MSB, buffer[1] = LSB
    uint16_t status = (buffer[0] << 8) | buffer[1];
    return status;
}

void LoraHandler::clearIrqStatus(uint16_t flags)
{
    // We must write two bytes telling the radio which IRQ bits to clear
    uint8_t buffer[2];
    buffer[0] = (uint8_t)(flags >> 8);
    buffer[1] = (uint8_t)(flags & 0xFF);

    Radio.WriteBuffer(SX126X_CLR_IRQ_STATUS, buffer, 2);
}

void LoraHandler::OnTxDone(void)
{
    if (instance)
    {
        Serial.println("OnTxDone");
        Radio.Rx(0);
        // Add logic if you want to repeat sends or handle post-send events
    }
}

void LoraHandler::OnTxTimeout(void)
{
    if (instance)
    {
        Serial.println("OnTxTimeout");
        Radio.Rx(0);
        // Handle timeout if necessary
    }
}

void LoraHandler::OnRxError(void)
{
    Serial.println("OnRxError");
    // Radio.Standby();

    uint16_t irqStatus = readIrqStatus();

    Serial.print("IRQ Status: 0x");
    Serial.println(irqStatus, HEX);

    // Clear the IRQ status so it's not misread next time
    clearIrqStatus(irqStatus);

    // The SX126x datasheet (and RAK driver) define bits like:
    //   0x20 = CRC error
    //   0x40 = Header error
    //   0x10 = SyncWord error
    //   etc.
    // Let's check a couple of common ones:

    if (irqStatus & 0x20)
    {
        Serial.println(" -> CRC error detected");
    }
    if (irqStatus & 0x40)
    {
        Serial.println(" -> Header error detected (wrong length/SF/bandwidth?)");
    }
    if (irqStatus & 0x10)
    {
        Serial.println(" -> SyncWord error detected");
    }

    // Re-enter receive mode
    Radio.Rx(0);
}

void LoraHandler::OnRxTimeout(void)
{
    Serial.println("OnRxTimeout");
    // Just put radio back in RX mode to keep listening
    Radio.Rx(0);
}

void LoraHandler::SerializeJSON(MessageType msgType)
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
        doc["rssi"] = receivedPacket.rssi;
        doc["snr"] = receivedPacket.snr;
        doc["rBatt"] = 27;                   // Receiver battery
        doc["hBatt"] = receivedPacket.hBatt; // Harness battery
    }
    else if (msgType == MSG_ACKNOWLEDGEMENT)
    {
        doc["ack"] = receivedPacket.ack;
        doc["rssi"] = receivedPacket.rssi;
        doc["snr"] = receivedPacket.snr;
    }
    else if (msgType == MSG_BUZZER)
    {
        doc["buzzer"] = receivedPacket.buzzer;
        doc["rssi"] = receivedPacket.rssi;
        doc["snr"] = receivedPacket.snr;
    }
    else if (msgType == MSG_LED)
    {
        doc["r"] = receivedPacket.r;
        doc["g"] = receivedPacket.g;
        doc["b"] = receivedPacket.b;
        doc["rssi"] = receivedPacket.rssi;
        doc["snr"] = receivedPacket.snr;
    }
    else if (msgType == MSG_RB_LED)
    {
        doc["rbLed"] = receivedPacket.rbLed; // rainbow led
        doc["rssi"] = receivedPacket.rssi;
        doc["snr"] = receivedPacket.snr;
    }
    else if (msgType == MSG_PWR_MODE)
    {
        doc["mode"] = receivedPacket.mode;
        doc["rssi"] = receivedPacket.rssi;
        doc["snr"] = receivedPacket.snr;
    }

    char buffer[200];
    size_t n = serializeJson(doc, buffer, sizeof(buffer));
    memset(RcvBuffer, 0, sizeof(RcvBuffer)); // Wipes entire buffer
    memcpy(RcvBuffer, buffer, n);            // Copy new payload
    RcvBuffer[n] = '\0';                     // Null-terminate the new data
}

void LoraHandler::OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    //wake device from sleep
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // xSemaphoreGiveFromISR(wakeSemaphore, &xHigherPriorityTaskWoken);
    // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // delay(10);
    OnRxToJSON(payload, rssi, snr);
    SerializeJSON(receivedPacket.msgType);
    Radio.Rx(RX_TIMEOUT_VALUE);
    packetReceived = true;
    Serial.println("Received Packet");
}

void LoraHandler::OnRxToJSON(uint8_t *payload, int16_t rssi, int8_t snr)
{
    // Now parse with ArduinoJson
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, (char *)payload);
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
            receivedPacket.rssi = rssi;
            receivedPacket.snr = snr;
        }
        if (receivedPacket.msgType == MSG_RB_LED)
        {
            receivedPacket.rbLed = doc["rbLed"]; // rainbow led
            receivedPacket.rssi = rssi;
            receivedPacket.snr = snr;
        }
        if (receivedPacket.msgType == MSG_PWR_MODE)
        {
            receivedPacket.mode = doc["mode"];
            receivedPacket.rssi = rssi;
            receivedPacket.snr = snr;
        }
        if (receivedPacket.msgType == MSG_BUZZER)
        {
            receivedPacket.buzzer = doc["buzzer"];
            receivedPacket.rssi = rssi;
            receivedPacket.snr = snr;
        }
        if (receivedPacket.msgType == MSG_ACKNOWLEDGEMENT)
        {
            receivedPacket.ack = doc["ack"];
            receivedPacket.rssi = rssi;
            receivedPacket.snr = snr;
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
            receivedPacket.rssi = rssi;
            receivedPacket.snr = snr;
            receivedPacket.rBatt = 27;           // Receiver battery
            receivedPacket.hBatt = doc["hBatt"]; // Harness battery
        };
    }
    else
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
    }
}

void LoraHandler::begin()
{
    instance = this;

    // Initialize LoRa chip using RAK function
    lora_rak4630_init();

    // Initialize RadioEvents
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone; // Implement if receiving packets
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    RadioEvents.CadDone = NULL;

    // Initialize the Radio
    Radio.Init(&RadioEvents);

    // Set frequency and other parameters
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    loraInitialized = true;
    Serial.println("Starting Radio.Rx");
    Radio.Rx(RX_TIMEOUT_VALUE);
    Serial.println("LoRa initialized.");
}

void LoraHandler::update()
{
    // If you want to receive packets, put Radio in Rx mode and implement RxDone callback
    // Radio.Rx(0);
    // Currently, no reception code here.
}

// MSG_ALL_DATA = 0
void LoraHandler::SendJSON(double lat, double lon, uint8_t hour,
                           uint8_t min, uint8_t sec, uint8_t siv,
                           uint16_t hdop, double alt)
{
    if (!loraInitialized)
        return;
    StaticJsonDocument<200> doc;
    doc["msgType"] = MSG_ALL_DATA;
    doc["lat"] = lat;
    doc["lon"] = lon;
    doc["hour"] = hour;
    doc["min"] = min;
    doc["sec"] = sec;
    doc["siv"] = siv;
    doc["hdop"] = hdop;
    doc["alt"] = alt;

    // Defualt Values to always be sent
    doc["mode"] = receivedPacket.mode;
    doc["rbLed"] = receivedPacket.rbLed; // rainbow led
    doc["r"] = receivedPacket.r;
    doc["g"] = receivedPacket.g;
    doc["b"] = receivedPacket.b;
    doc["hBatt"] = receivedPacket.hBatt; // Harness battery

    char buffer[200];
    size_t n = serializeJson(doc, buffer, sizeof(buffer));
    sendPacket((uint8_t *)buffer, n);
}

// MSG_ACKNOWLEDGEMENT = 1
void LoraHandler::SendJSON(MessageType msgType)
{
    if (!loraInitialized)
        return;
    StaticJsonDocument<200> doc;
    switch (msgType)
    {
        case MSG_BUZZER:
            doc["msgType"] = MSG_BUZZER;
            doc["buzzer"] = receivedPacket.buzzer;
            break;
        case MSG_LED:
            doc["msgType"] = MSG_LED;
            doc["r"] = receivedPacket.r;
            doc["g"] = receivedPacket.g;
            doc["b"] = receivedPacket.b;
            break;
        case MSG_RB_LED:
            doc["msgType"] = MSG_RB_LED;
            doc["rbLed"] = receivedPacket.rbLed; // rainbow led
            break;
        case MSG_PWR_MODE:
            doc["msgType"] = MSG_PWR_MODE;
            doc["mode"] = receivedPacket.mode;
            break;
        case MSG_ACKNOWLEDGEMENT:
            doc["msgType"] = MSG_ACKNOWLEDGEMENT;
            doc["ack"] = receivedPacket.ack;
            break;
    }
    char buffer[200];
    size_t n = serializeJson(doc, buffer, sizeof(buffer));
    sendPacket((uint8_t *)buffer, n);
}

void LoraHandler::sendPacket(uint8_t *buffer, uint8_t size)
{
    if (!loraInitialized)
        return;
    Radio.Send(buffer, size);
    Serial.print("Sent Packet: ");
    Serial.write((char *)buffer);
    Serial.println();
}

uint8_t *LoraHandler::GetRxPacket(void)
{
    return RcvBuffer;
}
