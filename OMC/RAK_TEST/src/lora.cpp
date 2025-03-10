/**
 * @file lora.cpp
 * @brief Implementation of LoRa handling functionality for the OzarkMountainCat project.
 *
 * This file implements functions to initialize the LoRa radio, handle transmit/receive events,
 * serialize and deserialize JSON packets, and manage LoRa events.
 */

 #include "lora.h"
 #include "main.h"


 // Global variables and objects
 /**
  * @brief Buffer for storing received LoRa packets.
  *
  * This buffer is used to hold the raw data received from the LoRa radio.
  */
 uint8_t RcvBuffer[200]; // Define the actual buffer
 
 /**
  * @brief Flag indicating whether a packet has been received.
  */
 bool packetReceived = false;
 
 /**
  * @brief Global instance of the ReceivedPacket structure.
  *
  * This instance stores the data extracted from received LoRa packets.
  */
 ReceivedPacket receivedPacket = {};
 
 /**
  * @brief Singleton instance pointer for LoraHandler.
  */
 LoraHandler *LoraHandler::instance = nullptr;
 
 /**
  * @brief Static variable for handling radio events.
  *
  * RadioEvents_t is used to store callback functions for various radio events.
  */
 static RadioEvents_t RadioEvents;
 
 #define SX126X_GET_IRQ_STATUS 0x15 /**< Opcode to get IRQ status from the SX126x radio. */
 #define SX126X_CLR_IRQ_STATUS 0x02 /**< Opcode to clear IRQ status from the SX126x radio. */
 
 /**
  * @brief Reads the IRQ status from the LoRa radio.
  *
  * This function sends a command to the radio to get the IRQ status,
  * reads two bytes from the radio, and then combines them into a 16-bit status value.
  *
  * @return uint16_t The IRQ status read from the radio.
  */
 uint16_t LoraHandler::readIrqStatus()
 {
     uint8_t buffer[2] = {0};
     // Read command: opcode, destination buffer, and number of bytes to read.
     Radio.ReadBuffer(SX126X_GET_IRQ_STATUS, buffer, 2);
 
     // Combine two bytes into a single 16-bit status (MSB first).
     uint16_t status = (buffer[0] << 8) | buffer[1];
     return status;
 }
 
 /**
  * @brief Clears specified IRQ status flags in the LoRa radio.
  *
  * This function writes two bytes to the radio to clear the IRQ status bits indicated by the flags.
  *
  * @param flags A 16-bit value representing the IRQ status flags to clear.
  */
 void LoraHandler::clearIrqStatus(uint16_t flags)
 {
     // Prepare two-byte buffer to specify which IRQ bits to clear.
     uint8_t buffer[2];
     buffer[0] = (uint8_t)(flags >> 8);
     buffer[1] = (uint8_t)(flags & 0xFF);
 
     Radio.WriteBuffer(SX126X_CLR_IRQ_STATUS, buffer, 2);
 }
 
 /**
  * @brief Callback function called when a LoRa transmission completes successfully.
  *
  * This function is invoked by the radio when a transmission is done.
  * It prints a message and then sets the radio back to receive mode.
  */
 void LoraHandler::OnTxDone(void)
 {
     if (instance)
     {
         Serial.println("OnTxDone");
         // Additional post-transmission logic can be added here.
     }
     Radio.Rx(0); // Set radio to RX mode.
 }
 
 /**
  * @brief Callback function called when a LoRa transmission times out.
  *
  * This function is invoked when a TX timeout occurs.
  * It prints a message and resets the radio to receive mode.
  */
 void LoraHandler::OnTxTimeout(void)
 {
     if (instance)
     {
         Serial.println("OnTxTimeout");
         // Handle timeout logic if necessary.
     }
     Radio.Rx(0); // Set radio to RX mode.
 }
 
 /**
  * @brief Callback function called when a LoRa reception error occurs.
  *
  * This function is invoked when an RX error is detected.
  * It reads and prints the IRQ status, clears the error flags, and re-enters RX mode.
  */
 void LoraHandler::OnRxError(void)
 {
     Serial.println("OnRxError");
     // Radio.Standby(); // Optionally set radio to standby.
 
     uint16_t irqStatus = readIrqStatus();
 
     Serial.print("IRQ Status: 0x");
     Serial.println(irqStatus, HEX);
 
     // Clear the IRQ status.
     clearIrqStatus(irqStatus);
 
     // Check for common errors:
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
 
     // Re-enter receive mode.
     Radio.Rx(0);
 }
 
 /**
  * @brief Callback function called when a LoRa reception times out.
  *
  * This function is invoked when an RX timeout occurs.
  * It prints a message and re-enters RX mode.
  */
 void LoraHandler::OnRxTimeout(void)
 {
     Serial.println("OnRxTimeout");
     // Simply re-enter RX mode to keep listening.
     Radio.Rx(0);
 }
 
 /**
  * @brief Serializes the current receivedPacket data into a JSON packet.
  *
  * Depending on the message type provided, this function fills a JSON document with the appropriate fields,
  * serializes it into a buffer, and copies it to the global RcvBuffer.
  *
  * @param msgType The message type to serialize.
  */
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
         doc["rBatt"] = 27;                   // Receiver battery (hardcoded example)
         doc["hBatt"] = receivedPacket.hBatt;  // Harness battery
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
         doc["rbLed"] = receivedPacket.rbLed; // rainbow LED
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
     memset(RcvBuffer, 0, sizeof(RcvBuffer)); // Clear the entire RcvBuffer.
     memcpy(RcvBuffer, buffer, n);            // Copy the new JSON payload.
     RcvBuffer[n] = '\0';                       // Null-terminate the string.
 }
 
 void DIOInterruptHandler()
 {
   if (uxSemaphoreGetCount(wakeSemaphore) == 0)
   {
     xSemaphoreGiveFromISR(wakeSemaphore, pdFALSE);
     Serial.print("Lora After Give: ");
     Serial.println(uxSemaphoreGetCount(wakeSemaphore));
   }
   else
   {
     Serial.println("Lora: Semaphore already given!");
   }
 }

 /**
  * @brief Callback function called when a LoRa packet is received.
  *
  * This function is called when a LoRa packet is received (RX done).
  * It processes the payload by converting it to JSON, serializes it,
  * re-enables RX mode, sets the packetReceived flag, prints a message,
  * queues an event, and gives the wake semaphore.
  *
  * @param payload Pointer to the received payload buffer.
  * @param size Size of the received payload.
  * @param rssi Received Signal Strength Indicator.
  * @param snr Signal-to-Noise Ratio.
  */
 void LoraHandler::OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
 {
     DIOInterruptHandler();
     delay(10);
     OnRxToJSON(payload, rssi, snr);
     SerializeJSON(receivedPacket.msgType);
     Radio.Rx(RX_TIMEOUT_VALUE);
     packetReceived = true;
     Serial.println("Received Packet");
     queEvent();
}

 /**
  * @brief Queues a LoRa event based on the current message type in receivedPacket.
  *
  * This function checks the global receivedPacket.msgType and then sends corresponding
  * events into the commandQueue. It also sets the msgType in receivedPacket to MSG_WAKE_TIMER
  * after processing.
  */
 void LoraHandler::queEvent()
 {
   switch (receivedPacket.msgType)
   {
   case MSG_ALL_DATA:
     // No extra event queued.
     Serial.println("Que All Data");
     break;
   case MSG_ACKNOWLEDGEMENT:
     // No extra event queued.
     Serial.println("Que Acknowledgement");
     break;
   case MSG_BUZZER:
     eventType = EVENT_ACKNOWLEDGEMENT;
     xQueueSend(commandQueue, &eventType, 0);
     eventType = EVENT_BUZZER;
     xQueueSend(commandQueue, &eventType, 0);
     Serial.println("Que Buzzer");
     break;
   case MSG_LED:
     eventType = EVENT_ACKNOWLEDGEMENT;
     xQueueSend(commandQueue, &eventType, 0);
     eventType = EVENT_LED;
     xQueueSend(commandQueue, &eventType, 0);
     Serial.println("Que LED");
     break;
   case MSG_RB_LED:
     eventType = EVENT_ACKNOWLEDGEMENT;
     xQueueSend(commandQueue, &eventType, 0);
     eventType = EVENT_RB_LED;
     xQueueSend(commandQueue, &eventType, 0);
     Serial.println("Que Rainbow LED");
     break;
   case MSG_PWR_MODE:
     eventType = EVENT_ACKNOWLEDGEMENT;
     xQueueSend(commandQueue, &eventType, 0);
     eventType = EVENT_PWR_MODE;
     xQueueSend(commandQueue, &eventType, 0);
     Serial.println("Que Power Mode");
     break;
   default:
     eventType = EVENT_WAKE_TIMER;
     xQueueSend(commandQueue, &eventType, 0);
     Serial.println("Que Wakeup Timer");
   }
   // Set the received packet message type to wake timer after queuing.
   receivedPacket.msgType = MSG_WAKE_TIMER;
 }
 
 /**
  * @brief Parses a received LoRa payload into a JSON object and updates the global receivedPacket.
  *
  * This function uses ArduinoJson to parse the payload and extracts fields according to the msgType.
  * It also updates rssi and snr values for the received packet.
  *
  * @param payload Pointer to the received payload buffer.
  * @param rssi Received Signal Strength Indicator.
  * @param snr Signal-to-Noise Ratio.
  */
 void LoraHandler::OnRxToJSON(uint8_t *payload, int16_t rssi, int8_t snr)
 {
     // Parse the payload using ArduinoJson.
     StaticJsonDocument<256> doc;
     DeserializationError error = deserializeJson(doc, (char *)payload);
     if (!error)
     {
         // Extract common fields.
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
             receivedPacket.rbLed = doc["rbLed"]; // rainbow LED
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
             receivedPacket.rBatt = 27;           // Receiver battery (hardcoded)
             receivedPacket.hBatt = doc["hBatt"];  // Harness battery
         }
     }
     else
     {
         Serial.print("JSON parse failed: ");
         Serial.println(error.c_str());
     }
 }
 
 

 /**
  * @brief Initializes the LoRa radio and its settings.
  *
  * This function initializes the LoRa chip (using the RAK provided library),
  * sets up the RadioEvents callbacks, configures the radio parameters (frequency, TX/RX configurations),
  * and starts the radio in RX mode.
  */
 void LoraHandler::begin()
 {
     instance = this;
 
     // Initialize LoRa chip using RAK function.
     lora_rak4630_init();
 
     // Initialize RadioEvents with callback functions.
     RadioEvents.TxDone = OnTxDone;
     RadioEvents.RxDone = OnRxDone; // For receiving packets.
     RadioEvents.TxTimeout = OnTxTimeout;
     RadioEvents.RxTimeout = OnRxTimeout;
     RadioEvents.RxError = OnRxError;
     RadioEvents.CadDone = NULL;
 
     // Initialize the Radio with the configured events.
     Radio.Init(&RadioEvents);
 
     // Set frequency and TX configuration.
     Radio.SetChannel(RF_FREQUENCY);
     Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                       LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                       LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                       true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
     // Set RX configuration.
     Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                       LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                       LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                       0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
 
     loraInitialized = true;
     Serial.println("Starting Radio.Rx");
     Radio.Rx(RX_TIMEOUT_VALUE);
     Serial.println("LoRa initialized.");
    //  detachInterrupt(LORA_DIO_PIN);
    //  delay(100);
    //  attachInterrupt(LORA_DIO_PIN, DIOInterruptHandler, FALLING);
    //  delay(100);

 }
 
 /**
  * @brief Updates LoRa reception state.
  *
  * Currently, this function is a placeholder. If you want to enable continuous reception,
  * you can call Radio.Rx(0) here.
  */
 void LoraHandler::update()
 {
     // To receive packets, put Radio in RX mode and implement the RxDone callback.
     // Radio.Rx(0);
     // Currently, no additional reception code is provided.
 }
 
 /**
  * @brief Sends a JSON packet containing all data (MSG_ALL_DATA) over LoRa.
  *
  * This function serializes GPS and other sensor data into a JSON packet and sends it via LoRa.
  *
  * @param lat Latitude value.
  * @param lon Longitude value.
  * @param hour Hour value.
  * @param min Minute value.
  * @param sec Second value.
  * @param siv Satellites in view.
  * @param hdop HDOP value.
  * @param alt Altitude value.
  */
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
 
     // Default values always sent.
     doc["mode"] = receivedPacket.mode;
     doc["rbLed"] = receivedPacket.rbLed; // Rainbow LED state.
     doc["r"] = receivedPacket.r;
     doc["g"] = receivedPacket.g;
     doc["b"] = receivedPacket.b;
     doc["hBatt"] = receivedPacket.hBatt; // Harness battery level.
 
     char buffer[200];
     size_t n = serializeJson(doc, buffer, sizeof(buffer));
     sendPacket((uint8_t *)buffer, n);
 }
 
 /**
  * @brief Sends a JSON packet for acknowledgement (MSG_ACKNOWLEDGEMENT) over LoRa.
  *
  * This function serializes an acknowledgement command into JSON and sends it via LoRa.
  *
  * @param ack Boolean flag for acknowledgement.
  */
 void LoraHandler::SendJSON(bool ack)
 {
     if (!loraInitialized)
         return;
     StaticJsonDocument<200> doc;
     doc["msgType"] = MSG_ACKNOWLEDGEMENT;
     doc["ack"] = ack;
 
     // Default values always sent.
     doc["mode"] = receivedPacket.mode;
     doc["rbLed"] = receivedPacket.rbLed; // Rainbow LED state.
     doc["r"] = receivedPacket.r;
     doc["g"] = receivedPacket.g;
     doc["b"] = receivedPacket.b;
     doc["hBatt"] = receivedPacket.hBatt; // Harness battery level.
 
     char buffer[200];
     size_t n = serializeJson(doc, buffer, sizeof(buffer));
     sendPacket((uint8_t *)buffer, n);
 }
 
 /**
  * @brief Sends a LoRa packet.
  *
  * This function sends the provided buffer (of given size) over the radio.
  *
  * @param buffer Pointer to the data buffer.
  * @param size Size of the data to send.
  */
 void LoraHandler::sendPacket(uint8_t *buffer, uint8_t size)
 {
     if (!loraInitialized)
         return;
     Radio.Send(buffer, size);
     Serial.print("Sent Packet: ");
     Serial.write((char *)buffer);
     Serial.println();
 }
 
 /**
  * @brief Returns a pointer to the received packet buffer.
  *
  * @return uint8_t* Pointer to the global RcvBuffer containing the last received packet.
  */
 uint8_t *LoraHandler::GetRxPacket(void)
 {
     return RcvBuffer;
 }
 