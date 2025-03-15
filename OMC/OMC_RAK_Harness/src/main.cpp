/**
 * @file main.cpp
 * @brief Main application file for the OzarkMountainCat project.
 *
 * This file sets up global objects, initializes peripherals,
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

QueHandler queHandler;
GPSHandler GPS;
SemaphoreHandle_t wakeSemaphore = NULL;
SoftwareTimer taskWakeupTimer;
LoraHandler Lora;
RGBHandler RGB;
BattHandler Batt;
BuzzerHandler Buzzer;
QueueHandle_t commandQueue;
EventType eventType;
BleHandler BLE;
bool wokeOnTimer = false;
bool initSetup = false;
int sleepTime = TIME_lIVE_TRACKING;

#define TICKS(ms) pdMS_TO_TICKS(ms)

/**
 * @brief Helper function to print the current stack high water mark.
 *
 * @param label A string label to identify where the check is performed.
 */
void printStackUsage(const char *label) {
  Serial.print(label);
  Serial.print(" - Stack High Water Mark: ");
  Serial.println(uxTaskGetStackHighWaterMark(NULL));
}

/**
 * @brief Forward declaration for queuing an event.
 */
void queEvent();

/**
 * @brief Timer callback for waking up the device.
 *
 * This function is called by the software timer to give the wake semaphore.
 */
void periodicWakeup(TimerHandle_t unused) {
  wokeOnTimer = true;
  if (uxSemaphoreGetCount(wakeSemaphore) == 0) {
    xSemaphoreGiveFromISR(wakeSemaphore, pdFALSE);
  }
}

/**
 * @brief Waits until a valid GPS fix is acquired.
 *
 * Periodically updates the GPS, processes the command queue,
 * and delays until both a fix is present and the number of satellites (SIV) is greater than 4.
 */
void waitForGPSFix() {
  while (!(GPS.hasFix())) {
    Serial.println("Waiting for GPS fix, processing queue...");
    GPS.update();
    queHandler.Que();
    vTaskDelay(TICKS(1000));
    printStackUsage("waitForGPSFix loop");
  }
  printStackUsage("After waitForGPSFix");
}

/**
 * @brief Handles the wake-up reason and prints debug steps.
 *
 * Determines if the wake was due to a timer or a LoRa event.
 */
void handleWakeUpReason() {
  if (wokeOnTimer) {
    Serial.println("Step 1:\n\nWoke up on timer");
    receivedPacket.msgType = MSG_WAKE_TIMER;
    wokeOnTimer = false;
  } else {
    Serial.println("Step 1:\n\nWoke up on LoRa");
  }
  printStackUsage("After handleWakeUpReason");
}

/**
 * @brief Activates the GPS based on the current mode and hardware signal.
 *
 * If the mode isn’t live tracking and a specific IO pin (WB_IO2) is LOW, the GPS is started.
 * Then the GPS data is updated.
 */
void activateGPS() {
  if (receivedPacket.mode != MODE_LIVE_TRACKING && digitalRead(WB_IO2) == LOW) {
    Serial.println("Step 2:\n\nWaking up GPS for fix...");
    GPS.begin();
  }
  Serial.println("Step 2:\n\nUpdating GPS data...");
  GPS.update();
  printStackUsage("After activateGPS");
}

/**
 * @brief Puts the device into sleep mode for the specified duration.
 *
 * The sleep duration is chosen based on the current operating mode.
 */
void Sleep() {
  switch (receivedPacket.mode) {
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
  taskWakeupTimer.stop();
  taskWakeupTimer.setPeriod(sleepTime);
  taskWakeupTimer.start();
  printStackUsage("After Sleep");
}

/**
 * @brief FreeRTOS task for power management and processing queued commands.
 *
 * This task is refactored to call helper functions to simplify the flow and reduce nesting.
 */
void powerManagementTask(void *pvParameters) {
  for (;;) {
    if (!initSetup) {
      Serial.println("Power Management Task running...");
      
      if (xSemaphoreTake(wakeSemaphore, portMAX_DELAY) == pdTRUE) {
        // Update battery status.
        receivedPacket.hBatt = Batt.mvToPercent(Batt.readVBatt());
        printStackUsage("After battery update");

        // Step 1: Handle wake-up reason.
        handleWakeUpReason();

        // Step 2: Activate and update GPS.
        activateGPS();

        // Step 3: Wait for GPS fix if not already acquired.
        if (!GPS.hasFix()) {
          waitForGPSFix();
        }
        Serial.println("Step 3:\n\nGPS fix acquired, processing queued events...");
        if (receivedPacket.msgType == MSG_WAKE_TIMER) {
          queEvent();
        }
        queHandler.Que();
        printStackUsage("After processing queued events");

        // Step 4: If not in live tracking, turn off GPS and then sleep.
        Serial.println("Step 4:");
        if (receivedPacket.mode != MODE_LIVE_TRACKING) {
          Serial.println("\nGPS fix acquired, turning off GPS...");
          GPS.gpsOff();
        }
        Sleep();
      }
    } else {
      // During initial setup, just process queued events.
      queHandler.Que();
    }
  }
}

/**
 * @brief Setup function.
 *
 * Initializes hardware, creates semaphores and queues, sets up the software timer,
 * and creates the power management task.
 */
void setup() {
  initSetup = true;
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  Wire.begin();
  time_t timeout = millis();
  Serial.begin(115200);

  // Wait for Serial connection with blue LED indication.
  while (!Serial) {
    if ((millis() - timeout) < 5000) {
      delay(100);
      digitalWrite(LED_BLUE, HIGH);
    } else {
      break;
    }
  }
  // Turn off blue LED once Serial is ready.
  digitalWrite(LED_BLUE, LOW);

  delay(2000);
  
  // Initialize GPS (retry until successful)
  while (!GPS.begin()) {
    Serial.println("Failed to initialize GPS! Trying again");
    delay(1000);
  }
  delay(2000);

  Lora.begin();
  RGB.begin();
  Batt.begin();

  // Create wake semaphore.
  wakeSemaphore = xSemaphoreCreateBinary();
  if (wakeSemaphore == NULL) {
    Serial.println("Failed to create wake semaphore!");
    while (1);
  }
  Serial.println("Giving wake semaphore");
  xSemaphoreGive(wakeSemaphore);

  // Create command queue.
  commandQueue = xQueueCreate(10, sizeof(eventType));
  if (commandQueue == NULL) {
    Serial.println("Failed to create command queue!");
    while (1);
  }

  // Create the power management task with a heap size of 2048.
  xTaskCreate(powerManagementTask, "PowerMgmt", 2048, NULL, 1, NULL);
  delay(1000);

  if (uxSemaphoreGetCount(wakeSemaphore) == 1) {
    Serial.println("Taking wake semaphore");
    xSemaphoreTake(wakeSemaphore, 10);
  }

  Serial.println("Initial Setup of Power Management Task Complete");
  initSetup = false;
  
  receivedPacket.mode = MODE_LIVE_TRACKING;
  Serial.println("Setup complete.");

  // Start the wakeup timer with an initial period of 15 seconds.
  taskWakeupTimer.begin(15000, periodicWakeup);
  taskWakeupTimer.start();
  
  printStackUsage("End of setup");
}

/**
 * @brief Main loop function.
 */
void loop() {
  vTaskDelay(portMAX_DELAY);
}

/**
 * @brief Queues an event based on the current message type.
 */
void queEvent() {
  switch (receivedPacket.msgType) {
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
