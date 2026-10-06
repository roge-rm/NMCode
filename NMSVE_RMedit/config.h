#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION 20261006

#define ENABLE_TRS true // set to false to use without hardware modification

// set defaults here
#define DEFAULTOUTPUT 0          // default output method (0 = TRS only, 1 = BT only, 2 = both)
#define DEFAULTROOT 0            // default root note
#define DEFAULTSCALE 0           // default scale (see scales[] in notes.cpp)
#define DEFAULTCHAN 9            // default MIDI channel
#define DEFAULTKNOB 0            // default knob function (0 = velocity, 1 = modulation, 2 = pan, 3 = expression)
#define DEFAULTVELOCITY 100      // default velocity
#define BLENAME "NMSVE-rm"       // name for BLE device
#define BUTTONHOLDTIME 1000      // number of ms to hold button for second function

#define NUMSCALES 16 // number of scales available (see scales[] in notes.cpp)
#define NUMPRESETS 11 // presets live on buttons 1-11, button 12 runs setup
#define MAXOCTAVE 8   // the fader selects octaves 0 through MAXOCTAVE

// physical buttons are numbered 1-12 (as used throughout comments and the README),
// but the buttons[]/buttonPins[] arrays are 0-indexed - use these constants instead of a raw
// index whenever a specific button is referenced by number, to avoid off-by-one mix-ups
enum
{
  BTN1 = 0,
  BTN2,
  BTN3,
  BTN4,
  BTN5,
  BTN6,
  BTN7,
  BTN8,
  BTN9,
  BTN10,
  BTN11,
  BTN12
};

#endif // CONFIG_H
