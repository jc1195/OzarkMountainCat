/**
 * @file main.cpp
 * @brief Main application file for the OzarkMountainCat project.
 *
 * This file sets up the global objects, handles initialization of peripherals,
 * and implements the main loop for power management, sleep cycles, and event queuing.
 */
#include "main.h"
#include "gps.h"
#include "buzzer.h"
#include "lora.h"
#include "bleHandler.h"
#include "batt.h"
#include "rgb.h"
#include <Wire.h>
// #include "board.h"

QueHandler queHandler;


/**
 * @brief Global instance of the GPSHandler class.
 *
 * Used for interfacing with the GPS module.
 */
GPSHandler GPS;

/**
 * @brief Semaphore used to wake the device upon LoRa reception or timer event.
 *
 * Initialized to NULL and created in setup().
 */
SemaphoreHandle_t wakeSemaphore = NULL;

/**
 * @brief SoftwareTimer object used to trigger wake-up events.
 */
SoftwareTimer taskWakeupTimer;

/**
 * @brief Global instance of the LoraHandler class.
 *
 * Used for sending and receiving LoRa packets.
 */
LoraHandler Lora;

RGBHandler RGB;

/**
 * @brief Global instance of the BattHandler class.
 *
 * Used for reading and processing battery voltage.
 */
BattHandler Batt;

/**
 * @brief Global instance of the BuzzerHandler class.
 *
 * Used for controlling the buzzer functionality.
 */
BuzzerHandler Buzzer;

/**
 * @brief Command queue used for queuing events that the system needs to process.
 */
QueueHandle_t commandQueue;

/**
 * @brief Global variable for event type.
 *
 * Holds the current event type for processing in the command queue.
 */
EventType eventType;

/**
 * @brief Global instance of the BleHandler class.
 *
 * Used for handling Bluetooth LE communications.
 */
BleHandler BLE;

/**
 * @brief Global flag indicating if the wake was due to a timer event.
 */
bool wokeOnTimer = false;

/**
 * @brief Global variable to store current sleep time.
 *
 * Initialized to TIME_lIVE_TRACKING.
 */
int sleepTime = TIME_lIVE_TRACKING;

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
  // Serial.print("periodicWakeup before give: ");
  // Serial.println(uxSemaphoreGetCount(wakeSemaphore));
  wokeOnTimer = true;

  if (uxSemaphoreGetCount(wakeSemaphore) == 0)
  {
    xSemaphoreGiveFromISR(wakeSemaphore, pdFALSE);
    // Serial.print("periodicWakeup After Give: ");
    // Serial.println(uxSemaphoreGetCount(wakeSemaphore));
  }
  else
  {
    // Serial.println("periodicWakeup: Semaphore already given!");
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
    Serial.println("Device going to sleep: 10 minutes");
    break;
  case MODE_POWER_SAVING:
    sleepTime = TIME_POWER_SAVING;
    Serial.println("Device going to sleep: 5 minutes");
    break;
  default:
    sleepTime = TIME_lIVE_TRACKING;
    Serial.println("Device going to sleep: 15 seconds");
    break;
  }
  Serial.println();
  wokeOnTimer = false;
  // Start the software timer with the given sleepTime and attach the periodicWakeup callback.
  taskWakeupTimer.stop();
  taskWakeupTimer.setPeriod(sleepTime);
  // taskWakeupTimer.begin(sleepTime, periodicWakeup, taskWakeupTimer.getID(), false);
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
    if (xSemaphoreTake(wakeSemaphore, portMAX_DELAY) == pdTRUE)
    {
      //Serial.println("Device woke up!");
      receivedPacket.hBatt = Batt.mvToPercent(Batt.readVBatt());

      // Wake up messages
      if (wokeOnTimer)
      {
        Serial.printf("Step 1: \n\n");
        Serial.println("Woke up on timer");
        receivedPacket.msgType = MSG_WAKE_TIMER;
        wokeOnTimer = false;
        queEvent();
        Serial.println();
        //Que();
      }
      else
      {
        Serial.printf("Step 1: \n\n");
        Serial.println("Woke up on Lora");
        Serial.println("Queing Lora Events");
        queEvent();
        Serial.println();
        //Que();
      }

      if (receivedPacket.mode != MODE_LIVE_TRACKING)
      {
        if (digitalRead(WB_IO2) == LOW)
        {
          Serial.printf("Step 2: \n\n");
          Serial.println("Waking up GPS for fix...");
          GPS.begin();
          Serial.println("Updating GPS data");
          Serial.println();
          GPS.update();
        }
      } else {
        Serial.printf("Step 2: \n\n");
        Serial.println("Updating GPS data");
        Serial.println();
        GPS.update();
      }
      
      // Wait for GPS fix and check queue.
      if (GPS.hasFix()){
        Serial.printf("Step 3: \n\n");
        GPS.update();
        Serial.println("GPS Aquired, checking que...");
        Serial.println();
        Serial.printf("Step 4: \n\n");
        queHandler.Que();
      } else {
        while (!GPS.hasFix())
        {
          Serial.printf("Step 3: \n\n");
          GPS.update();
          Serial.println("No GPS fix yet, checking que...");
          Serial.println();
          Serial.printf("Step 4: \n\n");
          queHandler.Que();
          vTaskDelay(TICKS(1000));
        }
      }

      // Turn GPS off when not in live tracking mode.
      if (receivedPacket.mode != MODE_LIVE_TRACKING)
      {
        Serial.printf("Step 5: \n\n");
        Serial.println("GPS fix acquired, turning off GPS...");
        Serial.println();
        GPS.gpsOff();
        Sleep();
      } else {
        Serial.printf("Step 5: \n\n");
        Serial.println();
        Sleep();
      }
      // Semaphore Event
    }
    // for loop
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
  pinMode(LED_BLUE, OUTPUT);

  Wire.begin();
  time_t timeout = millis();
  Serial.begin(115200);
  // Wait for Serial connection (for nrf52840 with native USB)
  // On nRF52840 the USB serial is not available immediately
  while (!Serial)
  {
    if ((millis() - timeout) < 5000)
    {
      delay(100);
      digitalWrite(LED_BLUE, HIGH);
    }
    else
    {
      break;
    }
  }

  delay(2000);

  while (!GPS.begin())
  {
    Serial.println("Failed to initialize GPS! Trying again");
    delay(1000);
  }

  delay(2000);

  Lora.begin();
  RGB.begin();
  Batt.begin();

  // Create the binary semaphore for wake-up events.
  wakeSemaphore = xSemaphoreCreateBinary();
  if (wakeSemaphore == NULL)
  {
    Serial.println("Failed to create wake semaphore!");
    while (1)
      ;
  }
  // Initially give the wake semaphore so that the device starts immediately.
  if (wakeSemaphore != NULL)
  {
    Serial.println(uxSemaphoreGetCount(wakeSemaphore));
    Serial.println("Giving wake semaphore");
    xSemaphoreGive(wakeSemaphore);
  }

  commandQueue = xQueueCreate(10, sizeof(eventType));
  if (commandQueue == NULL)
  {
    Serial.println("Failed to create command queue!");
    while (1)
      ;
  }

  // Create the power management task.
  xTaskCreate(powerManagementTask, "PowerMgmt", 2048, NULL, 1, NULL);
  delay(1000);

  if (wakeSemaphore != NULL)
  {
    Serial.println(uxSemaphoreGetCount(wakeSemaphore));
    if (uxSemaphoreGetCount(wakeSemaphore) == 1)
    {
      Serial.println("Taking wake semaphore");
      xSemaphoreTake(wakeSemaphore, 10);
    }
  }

  receivedPacket.mode = MODE_LIVE_TRACKING;

  Serial.println("Setup complete.");
  taskWakeupTimer.begin(15000, periodicWakeup);
  taskWakeupTimer.start();
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
  vTaskDelay(portMAX_DELAY);
}

/**
 * @brief Queues an event based on the current message type in receivedPacket.
 *
 * This function checks the message type in the global receivedPacket object
 * and sends corresponding events into the commandQueue for further processing.
 */
void queEvent()
{
  switch (receivedPacket.msgType)
  {
  case MSG_ALL_DATA:
    Serial.println("Que All Data");
    break;
  case MSG_ACKNOWLEDGEMENT:
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
    break;
  }
}
