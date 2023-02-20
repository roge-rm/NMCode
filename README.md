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
**1. On startup, the device prompts for the output mode.**

* 1 TRS Only
* 2 BT Only
* 3 BT + TRS
* 11 TRS Only, Default all other options
* 12 BT Only, Default all other options

The last two options are so that I can quickly skip through all settings and use the NMSVE right away. 
Default options are set near the top of the code.

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

Once booted the buttons are reassigned to whatever scale you chose, starting from the top left, with the root note of choice. The rotary knob will function as set above and the fader will choose the octave, as before. At any time the selection menu can be returned to by turning the rotary knob all the way to the left and then pressing buttons 9+10 or 11+12 together. This will allow you to reselect the scale/mode, root node, and rotary knob functions without restarting the device completely.

Cheers, thanks for reading.
<br>rm.

### Flashing Instructions
1. Download Arduino
2. Install ESP32 in Arduino
3. Load sketch, install required library (Bounce2)
4. Select board Firebeetle-ESP32
5. Plug in USB to serial FTDI adapter, select port in Arduino
6. Connect adapter to NMSVE - see <a href=https://github.com/roge-rm/NMCode/blob/main/images/pinout.png>pinout</a>
- Black/GND to GND on NMSVE
- Green/TX to RX on NMSVE
- White/RX to TX on NMSVE
7. Turn on NMSVE while holding boot pin to ground wire - POWER and CONNECT LEDs should be solid
8. Flash in Arduino



