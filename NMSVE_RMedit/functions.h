#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// leds.cpp
void initLEDs();
bool blinkTick(unsigned long intervalMs);
void flashLEDs(int flashes);

// input.cpp
void initButtons();
void updateButtons();
void updatePots();
bool settingsPosition();
void actionButtons();
int buttonChoice();
int buttonChoiceHeld(bool &held);

// notes.cpp
void setNotes();
void setupScale(bool skipsetup);

// midi.cpp
void initOutputs();
void readMIDI();
void midiON(int bNum);
void midiONChord(int bNum, int degree, int octaveShift);
void midiOFF(int bNum);
void sendCC(int CC, int value);

// presets.cpp
void selectPreset();
void setupMode();
void setupOutput();
void setupMIDIChan();
void setupRoot();
void setupKnob();
void savePreset();
void storePrefs(int presetNum);
void recallPrefs(int presetNum);

#endif // FUNCTIONS_H
