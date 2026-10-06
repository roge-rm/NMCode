#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <Preferences.h>

// persistent objects
extern Preferences presets;

// pin assignments
extern uint8_t faderPin;  // slider
extern uint8_t rotaryPin; // rotary knob
extern uint8_t led_Blue;
extern uint8_t led_Green;
extern uint8_t buttonPins[12];

// MIDI / scale state, shared between the input, notes, midi and presets modules
extern uint8_t valOutput;     // 0 = TRS only, 1 = BT only, 2 = both
extern uint8_t midiChan;
extern uint8_t valScale;
extern uint8_t valRoot;
extern uint8_t knobFunction;  // 0 = velocity, 1 = modulation, 2 = pan, 3 = expression
extern uint8_t velocityValue;
extern uint8_t currentOctave; // set by the fader
extern uint8_t knobValue;     // averaged knob position (0-127)
extern uint8_t chordMode;     // 0 = notes only, 1 = chords only (every button plays a chord), 2 = chord + note (buttons 1-6 play notes, 7-12 play chords)
extern uint8_t chordVoicing;  // 0 = triads (root+3rd+5th), 1 = seventh chords (root+3rd+5th+7th)

extern int buttonNotes[12]; // currently assigned button note
extern bool buttonIsOn[12]; // whether each button's note/chord is currently sounding

extern bool deviceConnected; // track whether bluetooth device is connected

// LED blink state, shared between the status LED in loop() and the setup-menu
// prompts, which all reuse the same blink timer
extern unsigned long ledTimer;
extern bool ledState;

#endif // GLOBALS_H
