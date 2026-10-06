#include <Arduino.h>

#include "config.h"
#include "globals.h"
#include "functions.h"

static int waitForButton(unsigned long blinkMs, int maxButton)
{ // blink the green LED until one of buttons 1..maxButton is pressed, return its index
  ledTimer = millis();
  updateButtons();
  int buttonNum = -1;
  while ((buttonNum < 0) || (buttonNum >= maxButton))
  {
    if (blinkTick(blinkMs))
      digitalWrite(led_Green, ledState);
    buttonNum = buttonChoice();
  }
  digitalWrite(led_Green, LOW);
  return buttonNum;
}

void selectPreset()
{
  ledTimer = millis();
  int buttonNum = -1;
  bool select = false;
  delay(50);
  flashLEDs(2);
  updateButtons();

  while (select == false)
  {
    if (blinkTick(1500))
      digitalWrite(led_Green, ledState);
    buttonNum = buttonChoice();
    switch (buttonNum)
    {
    case BTN1 ... BTN11: // select between presets 1 through 11
      recallPrefs(buttonNum);
      flashLEDs(buttonNum + 1);
      select = true;
      break;
    case BTN12: // initiate full setup
      flashLEDs(1);
      setupMode(); // run selection for output, channel, scale, root note, knob function
      select = true;
      break;
    }
  }

  digitalWrite(led_Green, LOW);
  digitalWrite(led_Blue, LOW);
}

void setupMode()
{
  setupOutput();     // choose TRS/BT output
  setupMIDIChan();   // choose MIDI channel
  setupScale(false); // choose scale
  setupRoot();       // choose root note
  setupKnob();       // choose knob function
  delay(50);
  flashLEDs(3);
}

void setupOutput()
{ // select output mode between TRS only, BT only, or both
#if ENABLE_TRS
  flashLEDs(1);
  valOutput = waitForButton(1200, BTN4); // buttons 1-3
#else
  valOutput = 1;
#endif
}

void setupMIDIChan()
{
  flashLEDs(2);
  midiChan = waitForButton(1000, 12) + 1; // add one to MIDI value as channel needs to be sent as 1-16 instead of 0-15
}

void setupRoot()
{
  flashLEDs(4);
  valRoot = waitForButton(600, 12);
  delay(50);
  updateButtons();
}

void setupKnob()
{ // select knob function - 1 = velocity, 2 = mod cc, 3 = pan cc, 4 = expression cc
  flashLEDs(5);
  knobFunction = waitForButton(400, BTN5); // buttons 1-4
  if (knobFunction != 0)
    velocityValue = DEFAULTVELOCITY; // knob no longer sets velocity, so don't leave it wherever it was last
  delay(50);
  updateButtons();
}

void savePreset()
{ // save preset to one of 11 preset slots
  ledTimer = millis();
  int buttonNum = -1;
  bool select = false;
  delay(50);
  flashLEDs(7);
  updateButtons();

  while (select == false)
  {
    if (blinkTick(750))
      digitalWrite(led_Green, ledState);
    buttonNum = buttonChoice();
    switch (buttonNum)
    {
    case BTN1 ... BTN11: // select between presets 1 through 11
      storePrefs(buttonNum);
      flashLEDs((buttonNum + 1) * 2);
      select = true;
      break;
    case BTN12: // press button 12 to exit and not save
      select = true;
      break;
    }
  }

  digitalWrite(led_Green, LOW);
}

// preset values are stored in NVS under numerical keys: (presetNum * 10) + field. Fields 1-5 use
// the same layout as nm2-RMe, 6-7 are the NMSVE-only output and knob settings
static void putPref(int presetNum, int field, uint8_t value)
{
  String key = String((presetNum * 10) + field); // convert numerical index to string as NVS keys cannot be integers
  presets.putUChar(key.c_str(), value);
}

static uint8_t getPref(int presetNum, int field, uint8_t defaultValue, uint8_t minValue, uint8_t maxValue)
{ // read a value, falling back to the default when it's missing or outside minValue..maxValue
  String key = String((presetNum * 10) + field);
  uint8_t value = presets.getUChar(key.c_str(), defaultValue);
  if ((value < minValue) || (value > maxValue))
    value = defaultValue;
  return value;
}

void storePrefs(int presetNum)
{ // write current settings to NVS
  putPref(presetNum, 1, midiChan);
  putPref(presetNum, 2, valScale);
  putPref(presetNum, 3, valRoot);
  putPref(presetNum, 4, chordMode);
  putPref(presetNum, 5, chordVoicing);
  putPref(presetNum, 6, valOutput);
  putPref(presetNum, 7, knobFunction);
}

void recallPrefs(int presetNum)
{ // recall settings from NVS
  midiChan = getPref(presetNum, 1, DEFAULTCHAN, 1, 12);
  valScale = getPref(presetNum, 2, DEFAULTSCALE, 0, NUMSCALES - 1);
  valRoot = getPref(presetNum, 3, DEFAULTROOT, 0, 11);
  chordMode = getPref(presetNum, 4, 0, 0, 2);
  chordVoicing = getPref(presetNum, 5, 0, 0, 1);
#if ENABLE_TRS
  valOutput = getPref(presetNum, 6, DEFAULTOUTPUT, 0, 2);
#else
  valOutput = 1;
#endif
  knobFunction = getPref(presetNum, 7, DEFAULTKNOB, 0, 3);
  if (knobFunction != 0)
    velocityValue = DEFAULTVELOCITY;

  setupScale(true); // set note values based on scale
}
