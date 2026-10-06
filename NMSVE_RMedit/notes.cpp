#include <Arduino.h>

#include "config.h"
#include "globals.h"
#include "functions.h"

// scale interval definitions (semitones between steps), same scales and order as nm2-RMe
static uint8_t noteInterval[11];
static const uint8_t scaleMajor[11] = {2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2};
static const uint8_t scaleNaturalMinor[11] = {2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2};
static const uint8_t scaleMelodicMinor[11] = {2, 1, 2, 2, 2, 2, 1, 2, 1, 2, 2};
static const uint8_t scaleHarmonicMinor[11] = {2, 1, 2, 2, 1, 3, 1, 2, 1, 2, 2};
static const uint8_t scaleDorian[11] = {2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2};
static const uint8_t scalePhrygian[11] = {1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2};
static const uint8_t scaleLydian[11] = {2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1};
static const uint8_t scaleMixolydian[11] = {2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2};
static const uint8_t scaleLocrian[11] = {1, 2, 2, 1, 2, 2, 2, 1, 2, 2, 1};
static const uint8_t scaleMinorPentatonic[11] = {3, 2, 2, 3, 2, 3, 2, 2, 3, 2, 3};
static const uint8_t scaleMajorPentatonic[11] = {2, 2, 3, 2, 3, 2, 2, 3, 2, 3, 2};
static const uint8_t scaleMajorBlues[11] = {2, 1, 1, 3, 2, 3, 2, 1, 1, 3, 2};
static const uint8_t scaleMinorBlues[11] = {3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 3};
static const uint8_t scaleAugmented[11] = {3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3};
static const uint8_t scaleDiminished[11] = {2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2};
static const uint8_t scaleNone[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// indexed by valScale (0-15), matches the README / the button-select order in setupScale()
static const uint8_t *const scales[NUMSCALES] = {
    scaleMajor, scaleNaturalMinor, scaleMelodicMinor, scaleHarmonicMinor,
    scaleDorian, scalePhrygian, scaleLydian, scaleMixolydian,
    scaleLocrian, scaleMinorPentatonic, scaleMajorPentatonic, scaleMajorBlues,
    scaleMinorBlues, scaleAugmented, scaleDiminished, scaleNone};

void setNotes()
{
  buttonNotes[0] = valRoot + (currentOctave * 12);
  for (int i = 1; i < 12; i++)
  {
    buttonNotes[i] = buttonNotes[i - 1] + noteInterval[i - 1];
  }
}

void setupScale(bool skipsetup)
{ // bool input to skip selection and assign scale
  // with only 12 buttons the 16 scales are split: a short press on buttons 1-12 picks scales 1-12,
  // holding buttons 1-4 for BUTTONHOLDTIME ms picks scales 13-16
  ledTimer = millis();
  updateButtons();
  int buttonNum;
  bool held;
  bool select = false;
  if (skipsetup == false)
    flashLEDs(3); // only flash when not loading a preset
  if (skipsetup == true)
    select = true;
  while (select == false)
  { // select scale
    if (blinkTick(800))
      digitalWrite(led_Green, ledState);
    buttonNum = buttonChoiceHeld(held);
    if (buttonNum > -1)
    {
      if (held && (buttonNum < (NUMSCALES - 12)))
        valScale = buttonNum + 12;
      else
        valScale = buttonNum;
      select = true;
    }
  }

  if (valScale >= NUMSCALES)
    valScale = DEFAULTSCALE;
  memcpy(noteInterval, scales[valScale], sizeof noteInterval);
}
