# NMSVE.rm
<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/rev2_1.jpg" width="700">

This is a fork of the NMSVE firmware by <a href=https://thisisnoiseinc.com/en-ca>this.is.Noise</a>

---

### Overview
I have added the following functionality:
* Selectable scales/modes (the same 16 scales as my <a href=https://github.com/roge-rm/nm2-RMe>nm2-RMe</a> firmware for the NM2)
* Selectable root note
* Chord modes (triads or 7th chords, chords only or chords + notes)
* Selectable knob function
* Output over TRS MIDI, BT (Bluetooth Low Energy), or both

These settings can be saved to 11 preset slots, each of which can be quickly recalled at boot.

*If you want to use this firmware without doing the hardware modification simply set **#define ENABLE_TRS** to **false** instead of **true** in `NMSVE_RMedit/config.h`. 
This will exclude any code related to sending data via TRS and will also skip the first selection step below (the device will boot staight to MIDI channel selection).*

<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/rev2_2.jpg" width="400">

---

### Assembly

#### Parts Required:
* <a href=https://thisisnoiseinc.com/en-ca/pages/noise-machine-nmsve>NMSVE</a>
* <a href=https://github.com/roge-rm/NMCode/tree/main/stl>3D printed housing</a>
* Female 3.5mm TRS jack
* 33 Ohm resistor
* 10 Ohm resistor
* Hookup wire
* USB to TTL adapter (for flashing) 

This project has now been updated with a second revision of a custom case. This case has been made larger than the original for improved ergonomics, room for a TRS MIDI out port, and a slightly larger battery. See <a href=https://github.com/roge-rm/NMCode/tree/main/stl>the STL folder</a> for both revisions of my modified/remade housing.

For my use the TRS jack is wired for TRS-A connections (<a href=https://github.com/roge-rm/NMCode/blob/main/images/pinout.png>see pinout</a>):
* Ground goes to ground on the NMSVE
* The sleeve is wired through a 33 Ohm resistor to the 3.3V pin
* The tip is wired through a 10 Ohm resistor to the TX pin

<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/rev2_4.jpg" width="400">

---

### Flashing Instructions
1. Download <a href=https://www.arduino.cc/en/software>Arduino</a>
2. <a href=https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/>Install ESP32</a> in Arduino
3. Open `NMSVE_RMedit/NMSVE_RMedit.ino` (the other files in that folder open as tabs), install required libraries (<a href=https://github.com/FortySevenEffects/arduino_midi_library>MIDI</a>, <a href=https://github.com/thomasfredericks/Bounce2>Bounce2</a>)
4. Select board Firebeetle-ESP32
5. Plug in USB to serial FTDI adapter, select port in Arduino
6. Connect adapter to NMSVE - see <a href=https://github.com/roge-rm/NMCode/blob/main/images/pinout.png>pinout</a>
- Black/GND to GND on NMSVE
- Green/TX to RX on NMSVE
- White/RX to TX on NMSVE
7. Turn on NMSVE while holding boot pin to ground wire - POWER and CONNECT LEDs should be solid
8. Flash in Arduino

Alternatively the project can be built and flashed with <a href=https://platformio.org/>PlatformIO</a> (`pio run -t upload`), which installs the libraries for you.

---

### Usage
**On startup, the device prompts you to select from one of the eleven presets or to run a full setup. Presets can be saved from any combination of configurations.**

* Buttons 1-11 select a preset
* Button 12 runs full setup

If presets have not been set up they will be launched with the default settings (configured in `NMSVE_RMedit/config.h`). Once adjusted they can be saved (see below).

**1. If you select the full setup, you will first be prompted to select the output options:**

* 1 TRS Only
* 2 BT Only
* 3 BT + TRS

**2. Next the device prompts for the MIDI channel. Buttons 1 through 12 select those channels.**

**3. Then the device prompts for the scale. Press buttons 1-12 to choose the first twelve scales, or _hold_ buttons 1-4 for a second to choose the last four:**

| Button | Press | Hold |
| --- | --- | --- |
| 1 | Major | Minor Blues |
| 2 | Natural Minor | Augmented |
| 3 | Melodic Minor | Diminished |
| 4 | Harmonic Minor | No Scale (Chromatic) |
| 5 | Dorian | |
| 6 | Phrygian | |
| 7 | Lydian | |
| 8 | Mixolydian | |
| 9 | Locrian | |
| 10 | Minor Pentatonic | |
| 11 | Major Pentatonic | |
| 12 | Major Blues | |

**4. After the scale is selected you are prompted for the root note. This is chosen using the same note layout as original firmware (starting with C at the top left).**

**5. Finally you choose the function of the rotary knob:**

* 1 Velocity (sent with note data)
* 2 Modulation CC (this is the current functionality in the stock firmware)
* 3 Pan CC
* 4 Expression CC

Once booted the buttons are reassigned to whatever scale you chose, starting from the top left, with the root note of choice. The rotary knob will function as set above and the fader will choose the octave, as before. If BT output is enabled the blue LED flashes until a device connects - TRS output works in the meantime.

***Please note:
When the NMSVE is turned on the ESP32 chip sends diagnostic debug info on the first serial port. As this is where the MIDI out is connected any devices attached will receive a burst of non MIDI data that may be interpreted strangely (often as a few notes and a transport start command). I suggest leaving the MIDI cable disconnected until after turning the NMSVE on to avoid this.***

---

### Changing Settings
If you would like to change settings without restarting the device, turn the knob to the _left_ all the way and slide the fader all the way to the _right_. No notes are sent in this position.

Then **hold** one of the following buttons for a second and release it to change settings or save/load presets:

* 1 Select MIDI channel
* 3 Chords only mode, 7th chords
* 4 Chords + notes mode, 7th chords
* 5 Chords only mode, triads
* 6 Chords + notes mode, triads
* 7 Select scale
* 8 Select root note
* 9 Load preset
* 10 Select knob function
* 11 Run full setup
* 12 Save preset

Selecting the chord mode that is already active turns chords back off.

Pressing 9 or 12 will prompt you to choose the slot to load/save preset. Press buttons 1 through 11 to select the required slot and your preset will be loaded or saved (press 12 to leave without saving). Presets store the output, MIDI channel, scale, root note, knob function and chord mode.

---

### Chord Modes

Chords are built from the selected scale, so they always stay in key:
* **Chords only** - buttons 1-6 play the chords on scale degrees 1-6, buttons 7-12 play the same chords an octave up
* **Chords + notes** - buttons 1-6 play single notes, buttons 7-12 play the chords on scale degrees 1-6 an octave below

Each mode can use triads (root, 3rd, 5th) or 7th chords (root, 3rd, 5th, 7th).

Cheers, I hope you enjoy.
<br>rm.
