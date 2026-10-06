/*
   NMCode by this.is.NOISE inc.
    https://github.com/thisisnoiseinc/NMCode

   Built upon:
    "BLE_MIDI Example by neilbags
    https://github.com/neilbags/arduino-esp32-BLE-MIDI
    Based on BLE_notify example by Evandro Copercini."

   RM.edit by rm
    https://github.com/roge-rm/NMCode

   The code is split into modules (same layout as nm2-RMe):
    config.h     - defaults and compile-time options
    globals.*    - pins and shared state
    input.cpp    - buttons, fader, knob and the alternative (settings) functions
    notes.cpp    - scales and note assignment
    midi.cpp     - TRS and BLE MIDI output, notes and chords
    presets.cpp  - setup prompts and preset storage
    leds.cpp     - LED prompts
*/

#include <Arduino.h>

#include "config.h"
#include "globals.h"
#include "functions.h"

void setup()
{
  presets.begin("nmsve", false); // initiate nmsve namespace to store/retrieve presets

  initButtons();
  initLEDs();

  delay(100);
  flashLEDs(1);

  updateButtons();

  selectPreset(); // choose one of the available presets or initiate setup mode
  initOutputs();  // start TRS and/or BLE MIDI depending on the chosen output

  delay(250);

  updatePots(); // get initial pot locations (used for setting octave/velocity)
  setNotes();   // set initial note values

  ledTimer = millis();
}

void loop()
{
  const int ledTime = 1000; // LED cycle time in ms

  bool usingBLE = (valOutput == 1) || (valOutput == 2);
  if (usingBLE && !deviceConnected)
  { // flash blue LED while waiting for a BLE connection (TRS keeps working in the meantime)
    if (blinkTick(ledTime))
      digitalWrite(led_Blue, ledState);
  }
  else
    digitalWrite(led_Blue, usingBLE); // solid blue while BLE is connected

  updatePots();    // check for changes to knob/fader
  updateButtons(); // check for button presses
  actionButtons(); // send MIDI messages / run alternative functions
  readMIDI();
}
