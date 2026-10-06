#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce2

#include "config.h"
#include "globals.h"
#include "functions.h"

// bounce objects for each of the 12 buttons. Every button reads HIGH while pressed, so the raw
// rose()/fell() edges are used throughout: rose() = pressed, fell() = released
static Bounce2::Button buttons[12];

// knob smoothing
static const int numReadings = 15;
static int readings[numReadings]; // the readings from the analog input
static int readIndex = 0;         // the index of the current reading
static int total = 0;             // the running total

void initButtons()
{
  for (int i = 0; i < 12; i++)
  {
    buttons[i].attach(buttonPins[i], INPUT);
    buttons[i].interval(5); // button debounce interval in ms
  }
}

void updateButtons()
{ // poll each button for state updates
  for (int i = 0; i < 12; i++)
    buttons[i].update();
}

static void potAverage()
{
  for (int p = 0; p < numReadings; p++)
  {
    total = total - readings[readIndex];                              // subtract the last reading
    readings[readIndex] = map(analogRead(rotaryPin), 0, 4095, 0, 127); // read from the sensor
    total = total + readings[readIndex];                              // add the reading to the total
    readIndex = (readIndex + 1) % numReadings;                        // advance, wrapping around at the end of the array
    delay(1);                                                         // delay in between reads for stability
  }
  knobValue = total / numReadings;
}

void updatePots()
{ // fader picks the octave, knob does whatever knobFunction says
  uint8_t newOctave = map(analogRead(faderPin), 0, 4095, 0, MAXOCTAVE);
  if (newOctave != currentOctave)
  { // change the octave and call for a note update
    currentOctave = newOctave;
    setNotes();
    digitalWrite(led_Green, currentOctave % 2); // green LED shows odd/even octave
  }

  static int lastKnobValue = -1;
  potAverage();
  if (knobValue != lastKnobValue)
  {
    lastKnobValue = knobValue;
    switch (knobFunction)
    {
    case 0: // velocity
      velocityValue = knobValue;
      break;
    case 1: // modulation
      if (!settingsPosition())
        sendCC(1, knobValue);
      break;
    case 2: // pan
      if (!settingsPosition())
        sendCC(10, knobValue);
      break;
    case 3: // expression
      if (!settingsPosition())
        sendCC(11, knobValue);
      break;
    }
  }
}

bool settingsPosition()
{ // alternative functions are accessible when the knob is turned all the way left and the fader slid all the way right
  return (knobValue == 0) && (currentOctave == MAXOCTAVE);
}

static bool heldRelease(int bNum)
{ // true on the release of a button that was held for at least BUTTONHOLDTIME ms
  return buttons[bNum].fell() && (buttons[bNum].previousDuration() > BUTTONHOLDTIME);
}

static void toggleChordMode(int mode, int voicing, int flashes)
{ // selecting the chord mode/voicing that is already active turns chords back off
  flashLEDs(flashes);
  bool active = (chordMode == mode && chordVoicing == voicing);
  chordMode = active ? 0 : mode;
  chordVoicing = voicing;
}

void actionButtons()
{ // perform actions based on buttons pressed/released or held
  for (int i = 0; i < 12; i++)
  {
    // always release a sounding note on button release, even if the knob/fader moved into the
    // settings position after the note started, otherwise the note hangs
    if (buttons[i].fell() && buttonIsOn[i])
      midiOFF(i);
  }

  if (!settingsPosition())
  { // read buttons and send corresponding MIDI messages
    if (velocityValue == 0)
      return; // nothing to play at zero velocity
    for (int i = 0; i < 12; i++)
    {
      if (buttons[i].rose())
      {
        if (chordMode == 1)
        { // chords only: buttons 1-6 play chords on degrees 1-6, buttons 7-12 play the same chords an
          // octave up, clamped so the top half never goes past the fader's top octave
          int half = i / 6;
          int halfOctave = min((int)currentOctave, MAXOCTAVE - 1) + half;
          midiONChord(i, i % 6, (halfOctave - currentOctave) * 12);
        }
        else if ((chordMode == 2) && (i >= BTN7))
          midiONChord(i, i - BTN7, -12); // chord + note: buttons 7-12 play chords one octave down so they ring under the notes played on 1-6
        else
          midiON(i);
      }
    }
  }
  else
  { // in the settings position buttons have second functions when they are held for BUTTONHOLDTIME ms
    if (heldRelease(BTN1))
    { // button 1 changes MIDI channel
      flashLEDs(1);
      setupMIDIChan();
    }
    if (heldRelease(BTN3)) // button 3 enables chord only mode, seventh chords
      toggleChordMode(1, 1, 5);
    if (heldRelease(BTN4)) // button 4 enables chord + note mode, seventh chords
      toggleChordMode(2, 1, 6);
    if (heldRelease(BTN5)) // button 5 enables chord only mode, triads
      toggleChordMode(1, 0, 4);
    if (heldRelease(BTN6)) // button 6 enables chord + note mode, triads
      toggleChordMode(2, 0, 3);
    if (heldRelease(BTN7))
    { // button 7 changes scale
      flashLEDs(1);
      setupScale(false);
      setNotes();
    }
    if (heldRelease(BTN8))
    { // button 8 changes root note
      flashLEDs(1);
      setupRoot();
      setNotes();
    }
    if (heldRelease(BTN9))
    { // button 9 loads a preset
      selectPreset();
      initOutputs();
      setNotes();
    }
    if (heldRelease(BTN10))
    { // button 10 changes knob function
      setupKnob();
    }
    if (heldRelease(BTN11))
    { // button 11 runs full setup
      setupMode();
      initOutputs();
      setNotes();
    }
    if (heldRelease(BTN12)) // button 12 saves preset
      savePreset();
  }
}

int buttonChoice()
{ // returns the button released this scan, or -1 if none
  updateButtons();

  for (int i = 0; i < 12; i++)
    if (buttons[i].fell())
      return i;

  return -1;
}

int buttonChoiceHeld(bool &held)
{ // like buttonChoice(), but also reports whether the button was held for BUTTONHOLDTIME ms
  int buttonNum = buttonChoice();
  held = (buttonNum > -1) && (buttons[buttonNum].previousDuration() > BUTTONHOLDTIME);
  return buttonNum;
}
