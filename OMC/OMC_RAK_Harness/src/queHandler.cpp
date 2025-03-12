#include "queHandler.h"
#include "lora.h"
#include "rgb.h"
#include "buzzer.h"
#include "gps.h"
#include "batt.h"

void QueHandler::Que()
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
      Serial.println("Processing command: Buzzer");
      Buzzer.begin();
      if (!receivedPacket.buzzer){
        Serial.println("Buzzer off");
        Buzzer.off();
      } else
      {
        Serial.println("Buzzer on");
        Buzzer.on();
      }
      break;
    case EVENT_PWR_MODE:
      Serial.println("Processing command: Change Power Mode");
      // Power Mode change is handled in the main loop.
      break;
    case EVENT_WAKE_TIMER:
      Serial.println("Processing command: Wake Timer Expired");
      // Routine wakeup: send a JSON packet with GPS and other data.
      Lora.SendJSON(GPS.getLatitude(), GPS.getLongitude(), GPS.getHour(), GPS.getMinute(), GPS.getSecond(), GPS.getSIV(), GPS.getHDOP(), GPS.getAltitude());
      break;
    default:
      break;
    }
    Serial.println("Que has been checked");
    Serial.printf("\n");
    // While loop
  }
  //vTaskDelay(10); // Delay for 1 tick (adjust as needed)
}
