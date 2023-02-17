# NMSVE.rm
<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/01.jpg" width="700">

This is a fork of the NMSVE firmware by this.is.Noise.

I have added the following functionality:
* Selectable scales/modes
* Selectable root note
* Selectable knob function
* Output over BT, TRS MIDI, or both

<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/02.jpg" width="400">

To support TRS MIDI output I have modified the original case (<a href=https://www.thingiverse.com/thing:5356460>thingiverse</a>) by making it 5mm thicker and hollowing out an area to fit the TRS jack and resistors. In this case the TRS jack is wired for TRS-A connections. Ground goes to ground on the NMSVE, the sleeve is wired through a 33 Ohm resistor to the 3.3V pin and the tip is wired through a 10 Ohm resistor to the TX pin. 

<img src="https://raw.githubusercontent.com/hunked/NMCode/main/images/03.jpg" width="400">

On startup, the device prompts for the output mode. 

* 1 TRS Only
* 2 BT Only
* 3 BT + TRS
* 11 TRS Only, Default all other options
* 12 BT Only, Default all other options

The last two options are so that I can quickly skip through all settings and use the NMSVE right away. 
Default options are set near the top of the code.

Next the device prompts for the MIDI channel. Buttons 1 through 12 select those channels.

Then the device prompts for the scale/mode:

* 1 Ionian Mode
* 2	Dorian Mode
* 3	Phrygian Mode
* 4	None
* 5	Lydian mode
* 6	Mixolydian Mode
* 7	Aeolian Mode
* 8	Locrian Mode
* 9 Pentatonic Scale
* 10 Blues Scale

After the scale is selected you are prompted for the root note, this is chosen using the same note layout as original firmware (starting with C at the top left).

Finally you choose the function of the rotary knob:

* 1 Velocity (sent with note data)
* 2 Modulation CC (this is the current functionality in the stock firmware)
* 3 Pan CC
* 4 Expression CC

Once booted the buttons are reassigned to whatever scale you chose, starting from the top left, with the root note of choice. The rotary knob will function as set above and the fader will choose the octave, as before. At any time the selection menu can be return to by turning the rotary knob all the way to the left and then pressing buttons 9+10 or 11+12 together.

This is getting to the point where I can't add anything else without making it too hard to remember what is going on. I am considering some kind of chord mode but I am not sure yet how I would implement it (or if I want to) so that may be a task for the future.

Cheers, thanks for reading.
rm.

