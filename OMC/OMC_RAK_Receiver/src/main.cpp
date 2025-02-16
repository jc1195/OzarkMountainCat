#include "main.h"
#include "Events.h"
#include "BleHandler.h"
#include "LoraHandler.h"
#include "RgbLed.h"
#include "GnssHandler.h"
#include "BuzzerHandler.h"
#include "PowerManager.h"
#include "batt.h"

// Global objects
BleHandler BLE;
LoraHandler loraHandler;
BattHandler Batt;
// RgbLed rgbLed(NEOPIXEL_PIN, 1);
// GnssHandler gnssHandler;
// BuzzerHandler buzzer(BUZZER_PIN);
// PowerManager powerManager;

uint32_t previousMillis = 0;

QueueHandle_t eventQueue = NULL;

// Track last broadcast time
static unsigned long lastBroadcast = 0;

void setup() {
    Serial.begin(115200);

    pinMode(LED_GREEN, OUTPUT);
    time_t serialTimeout = millis();
    while (!Serial)
    {
        digitalWrite(LED_GREEN, HIGH);
        delay(10); // for nrf52840 with native usb
        if ((millis() - serialTimeout) > 5000)
        {
            digitalWrite(LED_GREEN, LOW);
            break;
        }
    }

    eventQueue = xQueueCreate(10, sizeof(EventType));
    if (eventQueue == NULL) {
        Serial.println("Failed to create event queue!");
    } else {
        Serial.println("Event queue created successfully.");
    }

    receivedPacket.mode = MODE_LIVE_TRACKING;
    
    // rgbLed.begin();
    // gnssHandler.begin();
    // buzzer.begin();
    // bleHandler.begin();
    loraHandler.begin();
    BLE.begin();
    Batt.begin();

    // powerManager.begin(MODE_LIVE_TRACKING);
    // rgbLed.setColor(0, 0, 255); // Blue for startup
}

// void handleEvent(const SystemEvent &evt) {
//     switch (evt.type) {
//         case EVENT_TURN_ON_LED:
//             rgbLed.setColor(evt.r, evt.g, evt.b);
//             break;
//         case EVENT_TURN_OFF_LED:
//             rgbLed.setColor(0, 0, 0);
//             break;
//         case EVENT_BUZZER_ON:
//             buzzer.on();
//             break;
//         case EVENT_BUZZER_OFF:
//             buzzer.off();
//             break;
//         case EVENT_SEND_GNSS_DATA: {
//             gnssHandler.update();
//             if (gnssHandler.hasFix()) {
//                 loraHandler.sendStateAsJson(gnssHandler.getLatitude(), gnssHandler.getLongitude(), MODE_LIVE_TRACKING,False , False,gnssHandler.getHour(), gnssHandler.getMinute(), gnssHandler.getSecond(), gnssHandler.getSIV(), gnssHandler.getHDOP());
//             }
//             break;
//         }
//         case EVENT_CHANGE_MODE_LIVE:
//             powerManager.setMode(MODE_LIVE_TRACKING);
//             break;
//         case EVENT_CHANGE_MODE_POWER_SAVE:
//             powerManager.setMode(MODE_POWER_SAVING);
//             break;
//         case EVENT_CHANGE_MODE_EXTREME_SAVE:
//             powerManager.setMode(MODE_EXTREME_POWER_SAVING);
//             break;
//     }
// }

void loop(){
    //RadioEvents.RxDone
    if (packetReceived){
        char buffer[200];
        memcpy(buffer, RcvBuffer, sizeof(RcvBuffer));
        Serial.write(buffer);
        Serial.println();
        BLE.sendData(RcvBuffer, sizeof(RcvBuffer));
        packetReceived = false;
    }
    if ((millis() - previousMillis ) >= 10000)
    {
        
        previousMillis = millis();
        Serial.println("Looping");

    }
    if (bleReceived){
        Serial.println("Received packet from BLE");
        switch (receivedPacket.msgType){
            case MSG_LED:
                loraHandler.SendJSON(MSG_LED);
                break;
            case MSG_RB_LED:
               loraHandler.SendJSON(MSG_RB_LED);
                break;
            case MSG_BUZZER:
                loraHandler.SendJSON(MSG_BUZZER);
                break;
            case MSG_PWR_MODE:
                loraHandler.SendJSON(MSG_PWR_MODE);
                break;
            default:
                break;
        }
        bleReceived = false;
    }
}

// // void loop() {
    
//     // Check for events
//     SystemEvent evt;
//     if (xQueueReceive(eventQueue, &evt, 0) == pdTRUE) {
//         handleEvent(evt);
//     }

//     // Manage modes and timing
//     static unsigned long lastUpdate = 0;
//     DeviceMode mode = powerManager.getMode();

//     switch (mode) {
//         case MODE_LIVE_TRACKING:
//             if (millis() - lastUpdate > 15000) {
//                 // Instead of sending directly, queue an event to send data
//                 SystemEvent sendEvt = {EVENT_SEND_GNSS_DATA};
//                 xQueueSend(eventQueue, &sendEvt, 0);
//                 lastUpdate = millis();
//             }
//             break;
//         case MODE_POWER_SAVING:
//             // Sleep logic: wait 5 minutes or until woken by LoRa
//             // Placeholder: just delay here (you'll implement actual sleep later)
//             vTaskDelay(pdMS_TO_TICKS(5000)); // 5s for demo; replace with 5m
//             // After wake, queue an event to send data
//             {
//                 SystemEvent sendEvt = {EVENT_SEND_GNSS_DATA};
//                 xQueueSend(eventQueue, &sendEvt, 0);
//             }
//             break;
//         case MODE_EXTREME_POWER_SAVING:
//             // 10 minutes sleep logic
//             // Placeholder: just delay here (10s)
//             vTaskDelay(pdMS_TO_TICKS(10000));
//             {
//                 SystemEvent sendEvt = {EVENT_SEND_GNSS_DATA};
//                 xQueueSend(eventQueue, &sendEvt, 0);
//             }
//             break;
//     }

//     // Minimal delay to avoid busy loop
//     vTaskDelay(pdMS_TO_TICKS(100));
// }
