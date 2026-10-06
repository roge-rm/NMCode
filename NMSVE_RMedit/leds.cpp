#include <Arduino.h>

#include "globals.h"
#include "functions.h"

void initLEDs()
{
  pinMode(led_Blue, OUTPUT);
  pinMode(led_Green, OUTPUT);
}

bool blinkTick(unsigned long intervalMs)
{ // toggles the shared ledState/ledTimer blink clock once per intervalMs; returns true on the tick it flips.
  // Used by every "blink an LED while waiting for a button press" loop, since they never run concurrently.
  if (millis() <= (ledTimer + intervalMs))
    return false;
  ledState = !ledState;
  ledTimer = millis();
  return true;
}

void flashLEDs(int flashes)
{ // flash both LEDs together as a way to communicate confirmations and prompts
  for (int i = 0; i < flashes + 1; i++)
  {
    digitalWrite(led_Green, HIGH);
    digitalWrite(led_Blue, HIGH);
    delay(45);
    digitalWrite(led_Green, LOW);
    digitalWrite(led_Blue, LOW);
    delay(35);
  }
}
