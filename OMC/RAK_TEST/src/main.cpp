#include <main.h>
#include "lora.h"
#include "gps.h"
#include "buzzer.h"
#include "batt.h"
#include "rgb.h"
#include <Wire.h>

GPSHandler GPS;
SemaphoreHandle_t wakeSemaphore = NULL;
SoftwareTimer taskWakeupTimer;
LoraHandler Lora;
RGBHandler RGB;
BattHandler Batt;
BuzzerHandler Buzzer;
QueueHandle_t commandQueue;
EventType eventType;
bool wokeOnTimer = false;


void periodicWakeup(TimerHandle_t unused)
{
  // In an ISR context, indicate wake-up due to timer.
  Serial.print("periodicWakeup before give: ");
  Serial.println(uxSemaphoreGetCount(wakeSemaphore));
  wokeOnTimer = true;

  if (uxSemaphoreGetCount(wakeSemaphore) == 0)
  {
    xSemaphoreGiveFromISR(wakeSemaphore, pdFALSE);
    Serial.print("periodicWakeup After Give: ");
    Serial.println(uxSemaphoreGetCount(wakeSemaphore));
  }
  else
  {
    Serial.println("periodicWakeup: Semaphore already given!");
  }
}

void Sleep()
{
  // switch (receivedPacket.mode)
  // {
  // case MODE_EXTREME_POWER_SAVING:
  //   sleepTime = TIME_EXTREME_POWER_SAVING;
  //   Serial.println("Device going to sleep: 10 minutes");
  //   break;
  // case MODE_POWER_SAVING:
  //   sleepTime = TIME_POWER_SAVING;
  //   Serial.println("Device going to sleep: 5 minutes");
  //   break;
  // default:
  //   sleepTime = TIME_lIVE_TRACKING;
  //   Serial.println("Device going to sleep: 15 seconds");
  //   break;
  // }
  wokeOnTimer = false;
  // Start the software timer with the given sleepTime and attach the periodicWakeup callback.
  taskWakeupTimer.stop();
  taskWakeupTimer.setPeriod(300000);
  // taskWakeupTimer.begin(sleepTime, periodicWakeup, taskWakeupTimer.getID(), false);
  taskWakeupTimer.start();
}

void powerManagementTask(void *pvParameters)
{
  for (;;)
  {
    // Process any queued commands in a non-blocking manner.
    while (xQueueReceive(commandQueue, &eventType, 0) == pdTRUE)
    {
      Serial.println("Processing queued command...");
      switch (eventType)
      {
      case EVENT_ACKNOWLEDGEMENT:
        Serial.println("Processing command: send Acknowledgement");
        Lora.SendJSON(true);
        break;
      case EVENT_LED:
        Serial.println("Processing command: Handle RGB");
        RGB.setColor();
        break;
      case EVENT_RB_LED:
        Serial.println("Processing command: Turn on Rainbow LED");
        // Rainbow LED handling logic (to be implemented).
        break;
      case EVENT_BUZZER:
        Serial.println("Processing command: Stop Buzzer");
        Buzzer.begin();
        Buzzer.on();
        break;
      case EVENT_PWR_MODE:
        Serial.println("Processing command: Change Power Mode");
        // Power Mode change is handled in the main loop.
        break;
      case EVENT_WAKE_TIMER:
        Serial.println("Processing command: Wake Timer Expired");
        // Routine wakeup: send a JSON packet with GPS and other data.
        //Lora.SendJSON(GPS.getLatitude(), GPS.getLongitude(), GPS.getHour(), GPS.getMinute(), GPS.getSecond(), GPS.getSIV(), GPS.getHDOP(), GPS.getAltitude());

        Lora.SendJSON(43.5260706,-111.9634165,07,07,07,10,1,4910);
        break;
      default:
        break;
      }
      Serial.println("Que has been checked");
    }
    vTaskDelay(10); // Delay for 1 tick (adjust as needed)
  }
}

void setup() {
   // Create the command queue to hold up to 10 events.
   commandQueue = xQueueCreate(10, sizeof(eventType));
   if (commandQueue == NULL)
   {
     Serial.println("Failed to create command queue!");
     while (1)
       ;
   }

   // Create the power management task.
  xTaskCreate(powerManagementTask, "PowerMgmt", 512, NULL, 1, NULL);
  delay(1000);

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

  delay(1000);

  while (!GPS.begin()){
    Serial.println("Failed to initialize GPS! Trying again");
    delay(1000);
  }

  Lora.begin();
  RGB.begin();
  Batt.begin();
  
  // Create the binary semaphore for wake-up events.
  wakeSemaphore = xSemaphoreCreateBinary();
  if (wakeSemaphore == NULL)
  {
    Serial.println("Failed to create wake semaphore!");
    while (1);
  }
  // Initially give the wake semaphore so that the device starts immediately.
  if (wakeSemaphore != NULL)
  {
    Serial.println(uxSemaphoreGetCount(wakeSemaphore));
    Serial.println("Giving wake semaphore");
    xSemaphoreGive(wakeSemaphore);
  }

  if (wakeSemaphore != NULL)
  {
    Serial.println(uxSemaphoreGetCount(wakeSemaphore));
    Serial.println("Taking wake semaphore");
    xSemaphoreTake(wakeSemaphore, 10);
  }

  
  
  Serial.println("Setup complete.");
  taskWakeupTimer.begin(1000, periodicWakeup);
  taskWakeupTimer.start();
}

void loop() {
  if (xSemaphoreTake(wakeSemaphore, portMAX_DELAY) == pdTRUE)
  {
    if (wokeOnTimer){
      Serial.println("Woke on Timer");
      wokeOnTimer = false;
      GPS.update();
      Serial.print("LAT: " );
      Serial.println(GPS.getLatitude());
      Serial.print("LONG: " );  
      Serial.println(GPS.getLongitude());
      Serial.print("BAT: ");
      Serial.println(Batt.mvToPercent(Batt.readVBatt()));
      Serial.println("Going to sleep");
      Sleep();
    }
    else{
      GPS.update();
      Serial.println("Woke on Lora");
      Serial.print("LAT: " );
      Serial.println(GPS.getLatitude());
      Serial.print("LONG: " );  
      Serial.println(GPS.getLongitude());
      Serial.print("BAT: ");
      Serial.println(Batt.mvToPercent(Batt.readVBatt()));
      Serial.println("Going to sleep");
      Sleep();
    }

  }

}

