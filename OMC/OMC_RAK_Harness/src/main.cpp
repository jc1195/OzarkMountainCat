/**
 * @file main.cpp
 * @brief Main application file for the OzarkMountainCat project.
 *
 * This file sets up the global objects, handles initialization of peripherals,
 * and implements the main loop for power management, sleep cycles, and event queuing.
 */

#pragma once

#include "main.h"
#include "gps.h"
#include "buzzer.h"
#include "lora.h"
#include "bleHandler.h"
#include "batt.h"
#include <nrf_sdm.h> // Nordic SDK for deep sleep
#include "rgb.h"

/**
 * @brief Global instance of the GPSHandler class.
 *
 * Used for interfacing with the GPS module.
 */
GPSHandler GPS;

/**
 * @brief Global instance of the BuzzerHandler class.
 *
 * Used for controlling the buzzer functionality.
 */
BuzzerHandler Buzzer;

/**
 * @brief Global instance of the LoraHandler class.
 *
 * Used for sending and receiving LoRa packets.
 */
LoraHandler Lora;

/**
 * @brief Global instance of the BleHandler class.
 *
 * Used for handling Bluetooth LE communications.
 */
BleHandler BLE;

/**
 * @brief Global instance of the BattHandler class.
 *
 * Used for reading and processing battery voltage.
 */
BattHandler Batt;

/**
 * @brief Global variable for event type.
 *
 * Holds the current event type for processing in the command queue.
 */
EventType eventType;

/**
 * @brief Semaphore used to wake the device upon LoRa reception or timer event.
 *
 * Initialized to NULL and created in setup().
 */
SemaphoreHandle_t wakeSemaphore = NULL;

/**
 * @brief Command queue used for queuing events that the system needs to process.
 */
QueueHandle_t commandQueue;

RGBHandler RGB;

/**
 * @brief Global flag indicating if the GPS is active/working.
 */
bool status_gps = false;

/**
 * @brief Global variable to hold a previous millisecond timestamp.
 */
uint32_t previousMillis = 0;

/**
 * @brief Global counter for run iterations.
 */
int runCount = 0;

/**
 * @brief Global variable to store current sleep time.
 *
 * Initialized to TIME_lIVE_TRACKING.
 */
int sleepTime = TIME_lIVE_TRACKING;

/**
 * @brief Global flag indicating if the wake was due to a timer event.
 */
bool wokeOnTimer = false;

/**
 * @brief SoftwareTimer object used to trigger wake-up events.
 */
SoftwareTimer taskWakeupTimer;

/**
 * @brief Macro to convert milliseconds to FreeRTOS ticks.
 *
 * @param ms Number of milliseconds.
 */
#define TICKS(ms) pdMS_TO_TICKS(ms)

/**
 * @brief Forward declaration of the function to queue an event.
 */
void queEvent();

/**
 * @brief Timer callback function for waking up the device.
 *
 * This function is called by the timer and gives the wake semaphore so that
 * the main loop or task wakes up.
 *
 * @param unused Timer handle (not used).
 */
void periodicWakeup(TimerHandle_t unused)
{
  // In an ISR context, indicate wake-up due to timer.
  //Serial.print("periodicWakeup before give: ");
  //Serial.println(uxSemaphoreGetCount(wakeSemaphore));
  wokeOnTimer = true;

  if (uxSemaphoreGetCount(wakeSemaphore) == 0)
  {
    xSemaphoreGiveFromISR(wakeSemaphore, pdFALSE);
    //Serial.print("periodicWakeup After Give: ");
    //Serial.println(uxSemaphoreGetCount(wakeSemaphore));
  }
  else
  {
    //Serial.println("periodicWakeup: Semaphore already given!");
  }
}

/**
 * @brief Puts the device into sleep mode for the specified duration.
 *
 * This function starts the software timer with the given sleep time and then
 * blocks on the wake semaphore until an external event or timer callback occurs.
 *
 * @param sleepTime The duration (in milliseconds) for which the device should sleep.
 */
void Sleep()
{
  switch (receivedPacket.mode)
  {
  case MODE_EXTREME_POWER_SAVING:
    sleepTime = TIME_EXTREME_POWER_SAVING;
    //Serial.println("Device going to sleep: 10 minutes");
    break;
  case MODE_POWER_SAVING:
    sleepTime = TIME_POWER_SAVING;
    //Serial.println("Device going to sleep: 5 minutes");
    break;
  default:
    sleepTime = TIME_lIVE_TRACKING;
    //Serial.println("Device going to sleep: 15 seconds");
    break;
  }
  wokeOnTimer = false;
  // Start the software timer with the given sleepTime and attach the periodicWakeup callback.
  taskWakeupTimer.begin(sleepTime, periodicWakeup, taskWakeupTimer.getID(), false);
  taskWakeupTimer.start();
}

/**
 * @brief FreeRTOS task for processing power management and queued commands.
 *
 * This task runs continuously in an infinite loop, processing any events
 * from the commandQueue (non-blocking) and performing scheduled actions such as
 * sending LoRa packets based on the current power mode.
 *
 * @param pvParameters Pointer to task parameters (unused).
 */
void powerManagementTask(void *pvParameters)
{
  for (;;)
  {
    // Process any queued commands in a non-blocking manner.
    while (xQueueReceive(commandQueue, &eventType, 0) == pdTRUE)
    {
      //Serial.println("Processing queued command...");
      switch (eventType)
      {
      case EVENT_ACKNOWLEDGEMENT:
        //Serial.println("Processing command: send Acknowledgement");
        Lora.SendJSON(true);
        break;
      case EVENT_LED:
        //Serial.println("Processing command: Handle RGB");
        RGB.setColor();
        break;
      case EVENT_RB_LED:
        //Serial.println("Processing command: Turn on Rainbow LED");
        // Rainbow LED handling logic (to be implemented).
        break;
      case EVENT_BUZZER:
        //Serial.println("Processing command: Stop Buzzer");
        Buzzer.begin();
        Buzzer.on();
        break;
      case EVENT_PWR_MODE:
        //Serial.println("Processing command: Change Power Mode");
        // Power Mode change is handled in the main loop.
        break;
      case EVENT_WAKE_TIMER:
        //Serial.println("Processing command: Wake Timer Expired");
        // Routine wakeup: send a JSON packet with GPS and other data.
        Lora.SendJSON(GPS.getLatitude(), GPS.getLongitude(), GPS.getHour(), GPS.getMinute(), GPS.getSecond(), GPS.getSIV(), GPS.getHDOP(), GPS.getAltitude());
        break;
      default:
        break;
      }
      //Serial.println("Que has been checked");
    }
  }
}

/**
 * @brief Setup function.
 *
 * This function initializes hardware, creates semaphores and queues, sets up the
 * software timer, and initializes global objects such as GPS, LoRa, and battery monitoring.
 */
void setup()
{
  pinMode(LED_GREEN, OUTPUT);
  Wire.begin();
  //Serial2.begin(115200);
  
  ////Serial.begin(115200);
  
  
  //Serial.begin(115200);
  GPS.begin();

  //Wire.begin();
  // //Serial.begin(115200);

  // // Wait for //Serial connection (for nrf52840 with native USB)
  // uint32_t serialTimeout = 0;
  // while (!//Serial)
  // {
  //   digitalWrite(LED_GREEN, HIGH);
  //   delay(100);
  //   if ((millis() - serialTimeout) > 5000)
  //   {
  //     digitalWrite(LED_GREEN, LOW);
  //     break;
  //   }
  // }

  // Create the binary semaphore for wake-up events.
  wakeSemaphore = xSemaphoreCreateBinary();
  if (wakeSemaphore == NULL)
  {
    //Serial.println("Failed to create wake semaphore!");
    while (1)
      ;
  }

  // Create the command queue to hold up to 10 events.
  commandQueue = xQueueCreate(10, sizeof(eventType));
  if (commandQueue == NULL)
  {
    //Serial.println("Failed to create command queue!");
    while (1)
      ;
  }

  // Initially give the wake semaphore so that the device starts immediately.
  if (wakeSemaphore != NULL)
  {
    //Serial.println(uxSemaphoreGetCount(wakeSemaphore));
    //Serial.println("Giving wake semaphore");
    xSemaphoreGive(wakeSemaphore);
  }

  //  if (wakeSemaphore != NULL)
  //  {
  //    //Serial.println(uxSemaphoreGetCount(wakeSemaphore));
  //    //Serial.println("Taking wake semaphore");
  //    xSemaphoreTake(wakeSemaphore, 10);
  //  }

  // Create the power management task.
  xTaskCreate(powerManagementTask, "PowerMgmt", 2048, NULL, 1, NULL);

  // Set up the software timer to "wake" the device after TIME_lIVE_TRACKING milliseconds.

  // Set the initial power mode.
  receivedPacket.mode = MODE_LIVE_TRACKING;

  // Initialize peripherals.
  
  Lora.begin();
  
  // BLE.begin();
  Batt.begin();
  RGB.begin();
  RGB.off();

  //  taskWakeupTimer.begin(100, periodicWakeup, taskWakeupTimer.getID(), false);
  //  taskWakeupTimer.start();
  delay(1000);
  //Serial.println("Setup complete.");

  // Optionally, get an ADC sample for battery level.
  // receivedPacket.hBatt = Batt.mvToPercent(Batt.readVBatt());
}

/**
 * @brief Main loop function.
 *
 * This loop waits on the wake semaphore (blocking until an event occurs) and then
 * processes tasks based on the current power mode. It checks the GPS status, queues events,
 * and then calls Sleep() with the appropriate duration.
 */
void loop()
{
  if (xSemaphoreTake(wakeSemaphore, portMAX_DELAY) == pdTRUE)
  {
    // The device "wakes up" from an event.
    //Serial.println("Device woke up!");
    // receivedPacket.hBatt = Batt.readVBatt();
    receivedPacket.hBatt = Batt.mvToPercent(Batt.readVBatt());
    if (wokeOnTimer)
    {
      //Serial.println("Woke up on timer");
      receivedPacket.msgType = MSG_WAKE_TIMER;
      wokeOnTimer = false;
    }
    switch (receivedPacket.mode)
    {
    case MODE_LIVE_TRACKING:
      //Serial.println("Live Tracking Mode");
      GPS.update();
      while (!GPS.hasFix())
      {
        //Serial.println("No GPS fix yet...");
        GPS.update();
        vTaskDelay(TICKS(100));
      }
      queEvent();
      vTaskDelay(TICKS(10)); // Check queue for events.
      Sleep();               // Go back to sleep for 15 sec.
      break;
    case MODE_POWER_SAVING:
      sleepTime = TIME_POWER_SAVING;
      //Serial.println("Power Saving Mode");
      // GPS.begin();
      GPS.update();
      while (!GPS.hasFix())
      {
        //Serial.println("No GPS fix yet...");
        vTaskDelay(TICKS(100));
        GPS.update();
      }
      queEvent();
      vTaskDelay(TICKS(10)); // Check queue for events.
      // GPS.gpsOff();
      Sleep(); // Go back to sleep for 5 minutes.
      break;
    case MODE_EXTREME_POWER_SAVING:
      sleepTime = TIME_EXTREME_POWER_SAVING;
      //Serial.println("Extreme Power Saving Mode");
      // GPS.begin();
      GPS.update();
      while (!GPS.hasFix())
      {
        //Serial.println("No GPS fix yet...");
        vTaskDelay(TICKS(100));
        GPS.update();
      }
      queEvent();
      vTaskDelay(TICKS(10)); // Check queue for events.
      // GPS.gpsOff();
      Sleep(); // Go back to sleep for 10 minutes.
      break;
    default:
      vTaskDelay(TICKS(10)); // Check queue for events.
      //Serial.println("Unknown Mode");
      Sleep(); // Go back to sleep for 10 minutes.
      break;
    }
  }
}

/**
 * @brief Queues an event based on the current message type in receivedPacket.
 *
 * This function checks the message type in the global receivedPacket object
 * and sends corresponding events into the commandQueue for further processing.
 */
//  void queEvent()
//  {
//    switch (receivedPacket.msgType)
//    {
//    case MSG_ALL_DATA:
//      // No additional event; simply print.
//      //Serial.println("Que All Data");
//      break;
//    case MSG_ACKNOWLEDGEMENT:
//      // No additional event; simply print.
//      //Serial.println("Que Acknowledgement");
//      break;
//    case MSG_BUZZER:
//      //Serial.println("Processing command: Stop Buzzer");
//      Lora.SendJSON(true);
//      Buzzer.begin();
//      Buzzer.on();
//      break;
//    case MSG_LED:
//      //Serial.println("Processing command: Handle RGB");
//      Lora.SendJSON(true);
//      break;
//    case MSG_RB_LED:
//      //Serial.println("Processing command: Turn on Rainbow LED");
//      Lora.SendJSON(true);
//      break;
//    case MSG_PWR_MODE:
//      //Serial.println("Processing command: Change Power Mode");
//      Lora.SendJSON(true);
//      break;
//    default:
//      eventType = EVENT_WAKE_TIMER;
//      xQueueSend(commandQueue, &eventType, 0);
//      //Serial.println("Que Wakeup Timer");
//    }
//  }

// The alternate version of queEvent() is commented out below.

void queEvent()
{
  switch (receivedPacket.msgType)
  {
  case MSG_ALL_DATA:
    //Serial.println("Que All Data");
    break;
  case MSG_ACKNOWLEDGEMENT:
    //Serial.println("Que Acknowledgement");
    break;
  case MSG_BUZZER:
    eventType = EVENT_ACKNOWLEDGEMENT;
    xQueueSend(commandQueue, &eventType, 0);
    eventType = EVENT_BUZZER;
    xQueueSend(commandQueue, &eventType, 0);
    //Serial.println("Que Buzzer");
    break;
  case MSG_LED:
    eventType = EVENT_ACKNOWLEDGEMENT;
    xQueueSend(commandQueue, &eventType, 0);
    eventType = EVENT_LED;
    xQueueSend(commandQueue, &eventType, 0);
    //Serial.println("Que LED");
    break;
  case MSG_RB_LED:
    eventType = EVENT_ACKNOWLEDGEMENT;
    xQueueSend(commandQueue, &eventType, 0);
    eventType = EVENT_RB_LED;
    xQueueSend(commandQueue, &eventType, 0);
    //Serial.println("Que Rainbow LED");
    break;
  case MSG_PWR_MODE:
    eventType = EVENT_ACKNOWLEDGEMENT;
    xQueueSend(commandQueue, &eventType, 0);
    eventType = EVENT_PWR_MODE;
    xQueueSend(commandQueue, &eventType, 0);
    //Serial.println("Que Power Mode");
    break;
  default:
    eventType = EVENT_WAKE_TIMER;
    xQueueSend(commandQueue, &eventType, 0);
    //Serial.println("Que Wakeup Timer");
  }
}
