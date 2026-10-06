#include "globals.h"
#include "config.h"

Preferences presets;

// pin assignments
uint8_t faderPin = 36;
uint8_t rotaryPin = 39;
uint8_t led_Blue = 14;
uint8_t led_Green = 4;
#if UPWARD_BUTTONS
uint8_t buttonPins[12] = {27, 26, 35, 34, 19, 25, 22, 23, 16, 17, 18, 21}; // rows reversed - 1-4 on the bottom, 9-12 on top
#else
uint8_t buttonPins[12] = {16, 17, 18, 21, 19, 25, 22, 23, 27, 26, 35, 34};
#endif

// state variables
#ifdef FIXEDOUTPUT
uint8_t valOutput = FIXEDOUTPUT;
#else
uint8_t valOutput = DEFAULTOUTPUT;
#endif
uint8_t midiChan = DEFAULTCHAN;
uint8_t valScale = DEFAULTSCALE;
uint8_t valRoot = DEFAULTROOT;
uint8_t knobFunction = DEFAULTKNOB;
uint8_t velocityValue = DEFAULTVELOCITY; // MIDI velocity value
uint8_t currentOctave = 4;
uint8_t knobValue = 0;
uint8_t chordMode = 0; // chords, baby!
uint8_t chordVoicing = 0;

int buttonNotes[12];
bool buttonIsOn[12];

bool deviceConnected = false;

// timer variables
unsigned long ledTimer = 0;
bool ledState = false;
