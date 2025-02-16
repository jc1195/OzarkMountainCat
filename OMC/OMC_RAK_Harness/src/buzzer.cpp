#pragma once

#include "buzzer.h"

int tune[] = //List the frequencies according to the spectrum
{
  NTC5, NTC5, NTC6,
  NTCH1, NTC6, NTC5, NTC6, NTCH1, NTC6, NTC5,
  NTC3, NTC3, NTC3, NTC5,
  NTC6, NTC6, NTC5, NTCH3, NTCH3, NTCH2, NTCH1,
  NTCH2, NTCH1, NTCH2,
  NTCH3, NTCH3, NTCH2, NTCH3, NTCH2, NTCH1, NTCH2, NTCH1, NTC6,

  NTCH2, NTCH2, NTCH2, NTCH1, NTC6, NTC5,
  NTC6, NTC5, NTC5, NTCH1, NTC6, NTC5, NTC1, NTC3,
  NTC2, NTC1, NTC2,
  NTC3, NTC5, NTC5, NTC3, NTCH1, NTC7,
  NTC6, NTC5, NTC6, NTCH1, NTCH2, NTCH3,

  NTCH3, NTCH2, NTCH1, NTC5, NTCH1, NTCH2, NTCH3,

  NTCH2, NTC0, NTCH3, NTCH2,
  NTCH1, NTC0, NTCH2, NTCH1, NTC6, NTC0,

  NTCH2, NTC6, NTCH1, NTCH1, NTCH1, NTC6, NTC5, NTC3,
  NTC5,
  NTC5, NTC6, NTCH1, NTC7, NTC6,
  NTCH3, NTCH3, NTCH3, NTCH3, NTCH2, NTCH2, NTCH1,
  NTC6, NTCH3, NTCH2, NTCH1, NTCH2, NTCH1, NTC6,
  NTCH1,
};

float durt[] = //List the beats according to the notation
{
  HALF, QUARTER, QUARTER,
  WHOLE + HALF, HALF, HALF, QUARTER, QUARTER, HALF, HALF,
  WHOLE + WHOLE + WHOLE, HALF, QUARTER, QUARTER,
  WHOLE + HALF, HALF, HALF, HALF, QUARTER, QUARTER, HALF,
  WHOLE + WHOLE + WHOLE, HALF, HALF,
  HALF, HALF, HALF, QUARTER, QUARTER, HALF, QUARTER, QUARTER, HALF,
  HALF, HALF, HALF, QUARTER, QUARTER, WHOLE + WHOLE,
  HALF, HALF, HALF, HALF, HALF, HALF, HALF, HALF,
  WHOLE + WHOLE + WHOLE, HALF, HALF,


  WHOLE + HALF, HALF, HALF, HALF, HALF, HALF,
  WHOLE + HALF, HALF, WHOLE, HALF, QUARTER, QUARTER,
  WHOLE + HALF, HALF, HALF, HALF, HALF, QUARTER, QUARTER,
  WHOLE + WHOLE + WHOLE, HALF, QUARTER, QUARTER,
  WHOLE, HALF, QUARTER, QUARTER, WHOLE, WHOLE,
  HALF, HALF, HALF, HALF, WHOLE, QUARTER, QUARTER, HALF,
  WHOLE + WHOLE + WHOLE + WHOLE,
  HALF, HALF, HALF, HALF, WHOLE + WHOLE,
  HALF, HALF, HALF, HALF, WHOLE + HALF, QUARTER, QUARTER,
  WHOLE + HALF, HALF, WHOLE, QUARTER, QUARTER, QUARTER, QUARTER, WHOLE + WHOLE + WHOLE + WHOLE,

};

int length = 0;

void BuzzerHandler::begin() {
  //Serial.end();
  pinMode(PIN_BUZZER, OUTPUT);
  //HwPWM0.addPin(PIN_BUZZER);
  length = sizeof(tune) / sizeof(tune[0]); //Calculation length
}

void BuzzerHandler::on() {
    buzzerOn = true;
    for (int x = 0; x < length; x++)
    {
      tone(PIN_BUZZER, tune[x]);
      //HwPWM0.writePin(PIN_BUZZER,tune[x],false);
      delay(500 * durt[x]); //Here it is used to adjust the delay according to the beat. The 500 index can be adjusted by myself. In this music, I find that 500 is more suitable.
      vTaskDelay(pdMS_TO_TICKS(10));
      noTone(PIN_BUZZER);
    }
    off();
}

void BuzzerHandler::off() {
  digitalWrite(PIN_BUZZER, LOW);
  buzzerOn = false;
}

bool BuzzerHandler::isOn() {
  return buzzerOn;
}

