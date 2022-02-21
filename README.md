# NMSVE.rm
Noise Machine Straight Vibin Edition - RMedit.

This is a fork I (rm) created to change the functionality of the NMSVE a bit. I really enjoy the hardware and the BLE connectivity but I want to be able to input scales and use the rotary pot to adjust volume as I input notes into Sunvox. This is a bit of a mishmash of my own code from previous projects (I'm very familiar with the Bounce2 library so I used it for debouncing here) as well as the excellent work done by the this.is.Noise team.

On startup the device prompts for a MIDI channel, as before. Once the MIDI channel is selected the scale is selected from the following choices:

* 1	None
* 2	Major
* 3	Natural Minor
* 4	Harmonic Minor
* 5	Pentatonic Major
* 6	Pentatonic Minor
* 7	Whole Tone
* 8	Blues

Once the scale is selected you are prompted for the root note. This is chosen using the same note layout as original firmware. (starting with C at the top left).

Once booted the buttons are reassigned to whatever scale you chose, starting from the top left, with the root note of choice. The rotary knob will change the volume of each keypress and the fader will choose the octave, as before.

The code is still a work in progress but I think it is in a usable state so I am uploading it now. Cheers.

## Prerequisites:
 * https://github.com/espressif/arduino-esp32
 
## Notes
 Originally based off neilbags code:
 * https://github.com/neilbags/arduino-esp32-BLE-MIDI
 
 Use "ESP32 Pico Kit" as the board when uploading.
