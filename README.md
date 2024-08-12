# NMSVE.rm
<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/01.jpg" width="700">

This is a fork of the NMSVE firmware by <a href=https://thisisnoiseinc.com/en-ca>this.is.Noise</a>

I have added the following functionality:
* Selectable modes/scales
* Selectable root note
* Selectable knob function
* Output over TRS MIDI, BT (Bluetooth Low Energy), or both

<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/02.jpg" width="400">

To support TRS MIDI output I have modified the original case (<a href=https://www.thingiverse.com/thing:5356460>thingiverse</a>) by making it 5mm thicker and hollowing out an area to fit the TRS jack and resistors. In this case the TRS jack is wired for TRS-A connections. Ground goes to ground on the NMSVE, the sleeve is wired through a 33 Ohm resistor to the 3.3V pin and the tip is wired through a 10 Ohm resistor to the TX pin. 

If you want to use this firmware without doing the hardware modification simply set **#define ENABLE_TRS** to *false* instead of *true*. 
This will exclude any code related to sending data via TRS and will also skip the first selection step below (the device will boot staight to MIDI channel selection).

<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/03.jpg" width="400">

### Usage
**On startup, the device prompts you to select from one of the eight presets or to run a full setup. Presets can be saved from any combination of configurations using the full setup.**

* Buttons 1-8 select a preset
* Button 12 runs full setup

If presets have not been set up they will be launched with the default settings (configured near the top of the code). Once adjusted they can be saved (see below).

**1. If you select the full setup, you will first be prompted to select the output options:**

* 1 TRS Only
* 2 BT Only
* 3 BT + TRS

**2. Next the device prompts for the MIDI channel. Buttons 1 through 12 select those channels.**

**3. Then the device prompts for the scale/mode:**

* 1 Ionian Mode
* 2	Dorian Mode
* 3	Phrygian Mode
* 4	None (Chromatic)
* 5	Lydian Mode
* 6	Mixolydian Mode
* 7	Aeolian Mode
* 8	Locrian Mode
* 9 Pentatonic Major Scale
* 10 Penatonic Minor Scale
* 11 Blues Scale
* 12 Whole Tone Scale

**4. After the mode is selected you are prompted for the root note. This is chosen using the same note layout as original firmware (starting with C at the top left).**

**5. Finally you choose the function of the rotary knob:**

* 1 Velocity (sent with note data)
* 2 Modulation CC (this is the current functionality in the stock firmware)
* 3 Pan CC
* 4 Expression CC

Once booted the buttons are reassigned to whatever scale you chose, starting from the top left, with the root note of choice. The rotary knob will function as set above and the fader will choose the octave, as before.

If you would like to change settings without restarting the device, turn the knob to zero (counter clockwise) and slide the fader all the way to the left. Then you can press the following buttons to change settings or save/load presets:

* 1 Select MIDI channel
* 2 Select scale
* 3 Select root note
* 4 Select knob function
* 5 Run full setup
* 9 Load preset
* 12 Save preset

Pressing 9 or 12 will prompt you to choose the slot to load/save preset. Press buttons 1 through 8 to select the required slot and your preset will be loaded or saved!

Cheers, I hope you enjoy.
<br>rm.

### Flashing Instructions
1. Download <a href=https://www.arduino.cc/en/software>Arduino</a>
2. <a href=https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/>Install ESP32</a> in Arduino
3. Load sketch, install required libraries (<a href=https://github.com/FortySevenEffects/arduino_midi_library>MIDI</a>, <a href=https://github.com/thomasfredericks/Bounce2>Bounce2</a>)
4. Select board Firebeetle-ESP32
5. Plug in USB to serial FTDI adapter, select port in Arduino
6. Connect adapter to NMSVE - see <a href=https://github.com/roge-rm/NMCode/blob/main/images/pinout.png>pinout</a>
- Black/GND to GND on NMSVE
- Green/TX to RX on NMSVE
- White/RX to TX on NMSVE
7. Turn on NMSVE while holding boot pin to ground wire - POWER and CONNECT LEDs should be solid
8. Flash in Arduino



