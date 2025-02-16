#pragma once

#include "main.h"
#include "gps.h"
#include "buzzer.h"
#include "lora.h"
#include "bleHandler.h"
#include "batt.h"
#include <nrf_sdm.h> // Nordic SDK for deep sleep

GPSHandler GPS;
BuzzerHandler Buzzer;
LoraHandler Lora;
BleHandler BLE;
BattHandler Batt;
EventType eventType;

// Semaphore to wake up on LoRa reception
SemaphoreHandle_t wakeSemaphore = NULL;
QueueHandle_t commandQueue;

bool status_gps = false;
uint32_t previousMillis = 0;
int runCount = 0;
int sleepTime = TIME_lIVE_TRACKING;
bool wokeOnTimer = false;
SoftwareTimer taskWakeupTimer;

#define TICKS(ms) pdMS_TO_TICKS(ms)

// This function is called by the timer. It gives the semaphore so the loop wakes up.
void queEvent();

void periodicWakeup(TimerHandle_t unused)
{
  // In an ISR context, give the semaphore to wake the loop
  wokeOnTimer = true;
  xSemaphoreGiveFromISR(wakeSemaphore, pdFALSE);
  // xSemaphoreGive(wakeSemaphore);
}

void Sleep(uint32_t sleepTime)
{
  // In an ISR context, give the semaphore to wake the loop
  switch (sleepTime)
  {
  case TIME_EXTREME_POWER_SAVING:
    Serial.println("Device going to sleep: 10 minutes");
    break;
  case TIME_POWER_SAVING:
    Serial.println("Device going to sleep: 5 minutes");
    break;
  default:
    Serial.println("Device going to sleep: 15 seconds");
    break;
  }

  wokeOnTimer = false;
  // Clear any leftover tokens (if any)

  taskWakeupTimer.begin(sleepTime, periodicWakeup);
  taskWakeupTimer.start();
  // xSemaphoreTake(wakeSemaphore, portMAX_DELAY);
}

// Set LoRa event handling function
// LoRa Packet Interrupt Handler
void powerManagementTask(void *pvParameters)
{
  for (;;)
  {
    // Process any queued commands (non-blocking loop)
    while (xQueueReceive(commandQueue, &eventType, 0) == pdTRUE)
    {
      switch (eventType)
      {
      case EVENT_ACKNOWLEDGEMENT:
        Serial.println("Processing command: send Acknowledgement");
        Lora.SendJSON(true);
        break;
      case EVENT_LED:
        Serial.println("Processing command: Handle RGB");
        // DoNothing;
        // RGB is coming...
        break;
      case EVENT_RB_LED:
        Serial.println("Processing command: Turn on Rainbow LED");
        // DoNothing;
        // Rainbow LED is coming...
        break;
      case EVENT_BUZZER:
        Serial.println("Processing command: Stop Buzzer");
        Buzzer.begin();
        Buzzer.on();
        break;
      case EVENT_PWR_MODE:
        Serial.println("Processing command: Change Power Mode");
        // Do Nothing;
        // Power Mode gets handled in main loop.
        break;
      case EVENT_WAKE_TIMER:
        Serial.println("Processing command: Wake Timer Expired");
        // just a routine wakeup data send.
        Lora.SendJSON(GPS.getLatitude(), GPS.getLongitude(), GPS.getHour(), GPS.getMinute(), GPS.getSecond(), GPS.getSIV(), GPS.getHDOP(), GPS.getAltitude());
        break;
      default:
        break;
      }
    }
    // vTaskDelay(TICKS(100));
  }
}

void setup()
{
  pinMode(LED_GREEN, OUTPUT);

  Wire.begin();
  Serial.begin(115200);

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

  // Create the binary semaphore
  wakeSemaphore = xSemaphoreCreateBinary();
  if (wakeSemaphore == NULL)
  {
    Serial.println("Failed to create wake semaphore!");
    while (1)
      ;
  }

  // Create the command queue (able to hold, e.g., 10 commands)
  commandQueue = xQueueCreate(10, sizeof(eventType));
  if (commandQueue == NULL)
  {
    Serial.println("Failed to create command queue!");
    while (1)
      ;
  }

  // Initially give the semaphore so that the loop starts immediately
  if (wakeSemaphore != NULL)
  {
    Serial.println("Giving wake semaphore");
    xSemaphoreGive(wakeSemaphore);
  }

  if (wakeSemaphore != NULL)
  {
    Serial.println("Taking wake semaphore");
    xSemaphoreTake(wakeSemaphore, 10);
  }

  // Create the power management task
  // xTaskCreate(powerManagementTask, "PowerMgmt", 2048, NULL, 1, NULL);

  // // Set up the software timer to "wake" the device after SLEEP_TIME milliseconds
  taskWakeupTimer.begin(TIME_lIVE_TRACKING, periodicWakeup);
  taskWakeupTimer.start();

  receivedPacket.mode = MODE_LIVE_TRACKING;

  GPS.begin();
  Lora.begin();
  // BLE.begin();
  Batt.begin();

  // Get a single ADC sample and throw it away
  // receivedPacket.hBatt = Batt.mvToPercent(Batt.readVBatt());
}

void loop()
{
  if (xSemaphoreTake(wakeSemaphore, portMAX_DELAY) == pdTRUE)
  {
    // The device "wakes up" from lora.
    Serial.println("Device woke up!");
    receivedPacket.hBatt = Batt.readVBatt();
    if (wokeOnTimer)
    {
      Serial.println("Woke up on timer");
      receivedPacket.msgType = MSG_WAKE_TIMER;
      wokeOnTimer = false;
    }
    switch (receivedPacket.mode)
    {
    case MODE_LIVE_TRACKING:
      sleepTime = TIME_lIVE_TRACKING;
      Serial.println("Live Tracking Mode");
      GPS.update();
      while (!GPS.hasFix())
      {
        Serial.println("No GPS fix yet...");
        vTaskDelay(TICKS(1000));
        GPS.update();
      }
      queEvent();
      vTaskDelay(TICKS(100)); // check que for events.
      // xSemaphoreGive(wakeSemaphore);
      xSemaphoreTake(wakeSemaphore, 10);
      Sleep(receivedPacket.mode); // Go back to sleep for 15 sec.
      break;
    case MODE_POWER_SAVING:
      sleepTime = TIME_POWER_SAVING;
      Serial.println("Power Saving Mode");
      // GPS.begin();
      GPS.update();
      while (!GPS.hasFix())
      {
        Serial.println("No GPS fix yet...");
        vTaskDelay(TICKS(1000));
        GPS.update();
      }
      queEvent();
      vTaskDelay(TICKS(100)); // check que for events.
      // GPS.gpsOff();
      // xSemaphoreGive(wakeSemaphore);
      xSemaphoreTake(wakeSemaphore, 10);
      Sleep(TIME_POWER_SAVING); // Go back to sleep for 5 minutes.
      break;
    case MODE_EXTREME_POWER_SAVING:
      sleepTime = TIME_EXTREME_POWER_SAVING;
      Serial.println("Extreme Power Saving Mode");
      // GPS.begin();
      GPS.update();
      while (!GPS.hasFix())
      {
        Serial.println("No GPS fix yet...");
        vTaskDelay(TICKS(1000));
        GPS.update();
      }
      queEvent();
      vTaskDelay(TICKS(100)); // check que for events.
      // GPS.gpsOff();
      // xSemaphoreGive(wakeSemaphore);
      xSemaphoreTake(wakeSemaphore, 10);
      Sleep(TIME_EXTREME_POWER_SAVING); // Go back to sleep for 10 minutes.
      break;
    default:
      vTaskDelay(TICKS(100)); // check que for events.
      Serial.println("Unknown Mode");
      Sleep(TIME_EXTREME_POWER_SAVING); // Go back to sleep for 10 minutes.
      break;
    }
  }
}

void queEvent()
{
  switch (receivedPacket.msgType)
  {
  case MSG_ALL_DATA:
    // DoNothing;
    Serial.println("Que All Data");
    break;
  case MSG_ACKNOWLEDGEMENT:
    // DoNothing;
    Serial.println("Que Acknowledgement");
    break;
  case MSG_BUZZER:
    Serial.println("Processing command: Stop Buzzer");
    Lora.SendJSON(true);
    Buzzer.begin();
    Buzzer.on();
    break;
  case MSG_LED:
    Serial.println("Processing command: Handle RGB");
    Lora.SendJSON(true);
    // DoNothing;
    // RGB is coming...
    break;
  case MSG_RB_LED:
    Serial.println("Processing command: Turn on Rainbow LED");
    Lora.SendJSON(true);
    // DoNothing;
    // Rainbow LED is coming...

    break;
  case MSG_PWR_MODE:
    Serial.println("Processing command: Change Power Mode");
    Lora.SendJSON(true);
    // Do Nothing;
    // Power Mode gets handled in main loop.
    break;
  default:
    eventType = EVENT_WAKE_TIMER;
    xQueueSend(commandQueue, &eventType, 0);
    Serial.println("Que Wakeup Timer");
  }
}

// void queEvent()
// {
//   switch (receivedPacket.msgType)
//   {
//   case MSG_ALL_DATA:
//     // DoNothing;
//     Serial.println("Que All Data");
//     break;
//   case MSG_ACKNOWLEDGEMENT:
//     // DoNothing;
//     Serial.println("Que Acknowledgement");
//     break;
//   case MSG_BUZZER:
//     eventType = EVENT_ACKNOWLEDGEMENT;
//     xQueueSend(commandQueue, &eventType, 0);
//     eventType = EVENT_BUZZER;
//     xQueueSend(commandQueue, &eventType, 0);
//     Serial.println("Que Buzzer");
//     break;
//   case MSG_LED:
//     eventType = EVENT_ACKNOWLEDGEMENT;
//     xQueueSend(commandQueue, &eventType, 0);
//     eventType = EVENT_LED;
//     xQueueSend(commandQueue, &eventType, 0);
//     Serial.println("Que LED");
//     break;
//   case MSG_RB_LED:
//     eventType = EVENT_ACKNOWLEDGEMENT;
//     xQueueSend(commandQueue, &eventType, 0);
//     eventType = EVENT_RB_LED;
//     xQueueSend(commandQueue, &eventType, 0);
//     Serial.println("Que Rainbow LED");
//     break;
//   case MSG_PWR_MODE:
//     eventType = EVENT_ACKNOWLEDGEMENT;
//     xQueueSend(commandQueue, &eventType, 0);
//     eventType = EVENT_PWR_MODE;
//     xQueueSend(commandQueue, &eventType, 0);
//     Serial.println("Que Power Mode");
//     break;
//   default:
//     eventType = EVENT_WAKE_TIMER;
//     xQueueSend(commandQueue, &eventType, 0);
//     Serial.println("Que Wakeup Timer");
//   }
// }
