/*
   NMCode by this.is.NOISE inc.
    https://github.com/thisisnoiseinc/NMCode

   Built upon:
    "BLE_MIDI Example by neilbags
    https://github.com/neilbags/arduino-esp32-BLE-MIDI
    Based on BLE_notify example by Evandro Copercini."

   RM.edit by rm
    https://github.com/roge-rm/NMCode
*/

#define FIRMWARE_VERSION 20240811

#define ENABLE_TRS true  // set to false to use without hardware modification

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#define SERVICE_UUID "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>  // https://github.com/thomasfredericks/Bounce2
#include <MIDI.h>

#include <EEPROM.h>  // save and restore presets

#if ENABLE_TRS
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, DIN_MIDI);
#endif

// set defaults
#define DEFAULTINPUT 2  // default input method (0 = TRS only, 1 = BT only, 2 = both)
#define DEFAULTROOT 0   // default root note
#define DEFAULTSCALE 0  // default scale (same as below)
#define DEFAULTCHAN 9   // default MIDI channel
#define DEFAULTKNOB 0   // default knob function
#define baseEEPROM 100  // start address for EEPROM values

// pin assignments
int faderPin = 36;   // slider
int rotaryPin = 39;  // rotary Knob
int led_Blue = 14;
int led_Green = 4;
int buttonPins[12] = { 16, 17, 18, 21, 19, 25, 22, 23, 27, 26, 35, 34 };

// bool operators
bool deviceConnected = false;  // track whether bluetooth device is connected
bool stateChange = false;      // flag set to run note reassignment

// state variables
int valInput = DEFAULTINPUT;
int midiChan = DEFAULTCHAN;
int valScale = DEFAULTSCALE;
int valRoot = DEFAULTROOT;
int knobFunction = DEFAULTKNOB;  // change knob function: 0 = velocity, 1 = modulation, 2 = pan, 3 = expression
int velocityValue = 100;         // MIDI velocity value

// scale interval definitions (semitones between steps)
int noteInterval[11];
int modeIonian[11] = { 2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2 };
int modeDorian[11] = { 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2 };
int modePhrygian[11] = { 1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2 };
int scaleNone[11] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int modeLydian[11] = { 2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1 };
int modeMixolydian[11] = { 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2 };
int modeAeolian[11] = { 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2 };
int modeLocrian[11] = { 1, 2, 2, 1, 2, 2, 2, 1, 2, 2, 1 };
int scaleMajorPentatonic[11] = { 2, 2, 3, 2, 3, 2, 2, 3, 2, 3, 2 };
int scaleMinorPentatonic[11] = { 3, 2, 2, 3, 2, 3, 2, 2, 3, 2, 3 };
int scaleBlues[11] = { 3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 3 };
int scaleWholeTone[11] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };

int buttonNotes[12];   // currently assigned button note
int buttonPlayed[12];  // what note was played by each button last (in case the page of notes is changed while a note is being played; to prevent hung notes)

// timer variables
unsigned long ledTimer = 0;
const int ledTime = 1000;  // LED cycle time in ms
bool ledState = false;

// fader/rotary variables
int faderValue = 4;
const int numReadings = 15;
int readings[numReadings];  // the readings from the analog input
int readIndex = 0;          // the index of the current reading
int total = 0;              // the running total
int average1 = 0;           // average current state
int lastaverage1 = 0;       // average previous state

BLECharacteristic *pCharacteristic;

uint8_t midiPacket[] = {
  0x80,  // header
  0x80,  // timestamp, not implemented
  0x00,  // status
  0x3c,  // 0x3c == 60 == middle c
  0x00   // velocity
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

// create Bounce objects for each button
Bounce2::Button button1 = Bounce2::Button();
Bounce2::Button button2 = Bounce2::Button();
Bounce2::Button button3 = Bounce2::Button();
Bounce2::Button button4 = Bounce2::Button();
Bounce2::Button button5 = Bounce2::Button();
Bounce2::Button button6 = Bounce2::Button();
Bounce2::Button button7 = Bounce2::Button();
Bounce2::Button button8 = Bounce2::Button();
Bounce2::Button button9 = Bounce2::Button();
Bounce2::Button button10 = Bounce2::Button();
Bounce2::Button button11 = Bounce2::Button();
Bounce2::Button button12 = Bounce2::Button();


void setup() {
  EEPROM.begin(512);  // initiate ESP32 pseudo-EEPROM

  // button setup
  button1.attach(buttonPins[0], INPUT);
  button2.attach(buttonPins[1], INPUT);
  button3.attach(buttonPins[2], INPUT);
  button4.attach(buttonPins[3], INPUT);
  button5.attach(buttonPins[4], INPUT);
  button6.attach(buttonPins[5], INPUT);
  button7.attach(buttonPins[6], INPUT);
  button8.attach(buttonPins[7], INPUT);
  button9.attach(buttonPins[8], INPUT);
  button10.attach(buttonPins[9], INPUT);
  button11.attach(buttonPins[10], INPUT);
  button12.attach(buttonPins[11], INPUT);

  // button debounce interval in ms
  button1.interval(5);
  button2.interval(5);
  button3.interval(5);
  button4.interval(5);
  button5.interval(5);
  button6.interval(5);
  button7.interval(5);
  button8.interval(5);
  button9.interval(5);
  button10.interval(5);
  button11.interval(5);
  button12.interval(5);

  button9.setPressedState(LOW);
  button10.setPressedState(LOW);
  button11.setPressedState(LOW);
  button12.setPressedState(LOW);

  // needed for analog debounce?
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  // set up LED pins
  pinMode(led_Blue, OUTPUT);
  pinMode(led_Green, OUTPUT);

  delay(100);
  flashLEDs(1);

  updateButtons();

  selectPreset();  // choose one of the available presets or initiate setup mode

#if ENABLE_TRS
  if ((valInput == 0) || (valInput == 2)) {
    DIN_MIDI.begin(MIDI_CHANNEL_OMNI);
  }
#endif

  if ((valInput == 1) || (valInput == 2)) {
    BLEDevice::init("NMSVE.rm");

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
      BLEUUID(CHARACTERISTIC_UUID),
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE_NR);

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
  }

  delay(250);

  updatePots();  // get initial pot locations (used for setting octave/velocity)
  setNotes();    // set initial note values

  ledTimer = millis();
}

void selectPreset() {
  ledTimer = millis();
  int buttonNum = -1;
  bool select = false;
  delay(50);
  flashLEDs(2);

  while (select == false) {
    if (!(ledState) && (millis() > (ledTimer + 1500))) {
      digitalWrite(led_Green, HIGH);
      ledTimer = millis();
      ledState = true;
    } else if ((ledState) && (millis() > (ledTimer + 1500))) {
      digitalWrite(led_Green, LOW);
      ledTimer = millis();
      ledState = false;
    }
    buttonNum = buttonChoice();
    switch (buttonNum) {
      case 0 ... 7:  // select between presets 0 through 7
        recallEEPROM(buttonNum);
        flashLEDs(buttonNum + 1);
        select = true;
        break;
      case 11:  // initiate full setup
        flashLEDs(buttonNum + 1);
        setupMode();  // run selection for output, channel, root note, scale
        select = true;
        break;
    }
  }

  digitalWrite(led_Blue, LOW);
}

void setupMode() {
  setupInput();
  setupMIDI();
  setupScale(false);
  setupRoot();
  setupKnob();
  delay(50);
  flashLEDs(3);
}

void setupInput() {
  ledTimer = millis();
  updateButtons();

#if (ENABLE_TRS)
  flashLEDs(1);

  int buttonNum;
  bool select = false;
  while (select == false) {  // select output mode between TRS only, BT only, or both
    if (!(ledState) && (millis() > (ledTimer + 1200))) {
      digitalWrite(led_Green, HIGH);
      ledTimer = millis();
      ledState = true;
    } else if ((ledState) && (millis() > (ledTimer + 1200))) {
      digitalWrite(led_Green, LOW);
      ledTimer = millis();
      ledState = false;
    }
    buttonNum = buttonChoice();
    switch (buttonNum) {
      case 0:  // output only TRS
        valInput = 0;
        select = true;
        break;
      case 1:  // output only BT
        valInput = 1;
        select = true;
        break;
      case 2:  // output both
        valInput = 2;
        select = true;
        break;
    }
  }
#elif (!ENABLE_TRS)
  valInput = 1;
#endif
}

void setupMIDI() {
  ledTimer = millis();
  updateButtons();
  flashLEDs(2);
  int buttonNum;
  bool select = false;
  while (select == false) {  // select MIDI channel
    if (!(ledState) && (millis() > (ledTimer + 1000))) {
      digitalWrite(led_Green, HIGH);
      ledTimer = millis();
      ledState = true;
    } else if ((ledState) && (millis() > (ledTimer + 1000))) {
      digitalWrite(led_Green, LOW);
      ledTimer = millis();
      ledState = false;
    }
    buttonNum = buttonChoice();
    if (buttonNum > -1) {
      midiChan = buttonNum;  // set channel between 1-12 (instead of 0-11)
      select = true;
    }
  }
}

void setupScale(bool skipsetup) {  // bool input to skip selection and assign scale
  ledTimer = millis();
  updateButtons();
  int buttonNum;
  bool select = false;
  if (skipsetup == false) flashLEDs(3);  // only flash when not loading a preset
  if (skipsetup == true) select = true;
  while (select == false) {  // select scale
    if (!(ledState) && (millis() > (ledTimer + 800))) {
      digitalWrite(led_Green, HIGH);
      ledTimer = millis();
      ledState = true;
    } else if ((ledState) && (millis() > (ledTimer + 800))) {
      digitalWrite(led_Green, LOW);
      ledTimer = millis();
      ledState = false;
    }
    buttonNum = buttonChoice();
    if ((buttonNum > -1) && (buttonNum < 12)) {
      valScale = buttonNum;
      select = true;
    }
  }

  switch (valScale) {
    case 0:
      memcpy(noteInterval, modeIonian, sizeof noteInterval);
      break;
    case 1:
      memcpy(noteInterval, modeDorian, sizeof noteInterval);
      break;
    case 2:
      memcpy(noteInterval, modePhrygian, sizeof noteInterval);
      break;
    case 3:
      memcpy(noteInterval, scaleNone, sizeof noteInterval);
      break;
    case 4:
      memcpy(noteInterval, modeLydian, sizeof noteInterval);
      break;
    case 5:
      memcpy(noteInterval, modeMixolydian, sizeof noteInterval);
      break;
    case 6:
      memcpy(noteInterval, modeAeolian, sizeof noteInterval);
      break;
    case 7:
      memcpy(noteInterval, modeLocrian, sizeof noteInterval);
      break;
    case 8:
      memcpy(noteInterval, scaleMajorPentatonic, sizeof noteInterval);
      break;
    case 9:
      memcpy(noteInterval, scaleMinorPentatonic, sizeof noteInterval);
      break;
    case 10:
      memcpy(noteInterval, scaleBlues, sizeof noteInterval);
      break;
    case 11:
      memcpy(noteInterval, scaleWholeTone, sizeof noteInterval);
      break;
  }
}

void setupRoot() {
  ledTimer = millis();
  updateButtons();
  flashLEDs(4);
  int buttonNum;
  bool select = false;
  while (select == false) {  // select root note
    if (!(ledState) && (millis() > (ledTimer + 600))) {
      digitalWrite(led_Green, HIGH);
      ledTimer = millis();
      ledState = true;
    } else if ((ledState) && (millis() > (ledTimer + 600))) {
      digitalWrite(led_Green, LOW);
      ledTimer = millis();
      ledState = false;
    }
    buttonNum = buttonChoice();
    if ((buttonNum > -1) && (buttonNum < 12)) {
      valRoot = buttonNum;
      select = true;
      delay(50);
      updateButtons();
      buttonNum = -1;
    }
  }
}

void setupKnob() {
  ledTimer = millis();
  updateButtons();
  flashLEDs(5);
  int buttonNum;
  bool select = false;
  while (select == false) {  // select knob function - 1 = velocity, 2 = mod cc, 3 = pan cc, 4 = expression cc
    if (!(ledState) && (millis() > (ledTimer + 400))) {
      digitalWrite(led_Green, HIGH);
      ledTimer = millis();
      ledState = true;
    } else if ((ledState) && (millis() > (ledTimer + 400))) {
      digitalWrite(led_Green, LOW);
      ledTimer = millis();
      ledState = false;
    }
    digitalWrite(led_Green, HIGH);
    buttonNum = buttonChoice();
    if ((buttonNum > -1) && (buttonNum < 4)) {
      knobFunction = buttonNum;
      select = true;
      delay(50);
      updateButtons();
      buttonNum = -1;
    }
  }
}

void loop() {
  // if BLE is enabled, flash blue LED while waiting for connection
  if (((valInput == 1) || (valInput == 2)) && (deviceConnected == false)) {
    if (!(ledState) && (millis() > (ledTimer + ledTime))) {
      digitalWrite(led_Blue, HIGH);
      ledTimer = millis();
      ledState = true;
    } else if ((ledState) && (millis() > (ledTimer + ledTime))) {
      digitalWrite(led_Blue, LOW);
      ledTimer = millis();
      ledState = false;
    }
  }

  else {  // if BLE connected/no BLE, run loop
    if ((valInput == 1) || (valInput == 2)) digitalWrite(led_Blue, HIGH);
    updatePots();  // check for changes to pot/fader

    if (stateChange) {  // change octave if needed
      setNotes();
      stateChange = false;

      if (faderValue == 0 || faderValue == 2 || faderValue == 4 || faderValue == 6 || faderValue == 8) {
        digitalWrite(led_Green, LOW);
      } else {
        digitalWrite(led_Green, HIGH);
      }
    }

    updateButtons();  // check for button presses
    doMIDI();         // send any required MIDI messages

    if ((average1 == 0) && (faderValue == 0)) {  // alternative functions accessible when velocity and octave both turned to 0
      if (button1.released()) {                  // select channel
        setupMIDI();
        flashLEDs(1);
        updateButtons();
      } else if (button2.released()) {  // select scale
        setupScale(false);
        flashLEDs(2);
        updateButtons();
      } else if (button3.released()) {  // select root
        setupRoot();
        flashLEDs(3);
        updateButtons();
      } else if (button4.released()) {  // select knob function
        setupKnob();
        flashLEDs(5);
        updateButtons();
      } else if (button5.released()) {  // full setup
        setupMode();
        flashLEDs(4);
        updateButtons();
      } else if (button9.pressed()) {  // load preset
        selectPreset();
        flashLEDs(9);
        updateButtons();
      } else if (button12.released()) {  // save preset
        savePreset();
        flashLEDs(12);
        updateButtons();
      }
    }
  }

#if ENABLE_TRS
  if ((valInput == 0) || (valInput == 2)) DIN_MIDI.read();
#endif
}

void flashLEDs(int flashes) {
  for (int i = 0; i < flashes + 1; i++) {
    digitalWrite(led_Green, HIGH);
    digitalWrite(led_Blue, HIGH);
    delay(45);
    digitalWrite(led_Green, LOW);
    digitalWrite(led_Blue, LOW);
    delay(35);
  }
}

void updateButtons() {
  button1.update();
  button2.update();
  button3.update();
  button4.update();
  button5.update();
  button6.update();
  button7.update();
  button8.update();
  button9.update();
  button10.update();
  button11.update();
  button12.update();
}

void updatePots() {
  int newFaderValue = map(analogRead(faderPin), 0, 4095, 0, 8);
  if (faderValue != newFaderValue) {
    stateChange = true;
    faderValue = newFaderValue;
  }

  potAverage();

  if (average1 != lastaverage1) {
    lastaverage1 = average1;
    switch (knobFunction) {
      case 0:  // velocity
        velocityValue = average1;
        break;
      case 1:
        sendCC(1, average1);
        break;
      case 2:
        sendCC(10, average1);
        break;
      case 3:
        sendCC(11, average1);
        break;
    }
  }
}

void potAverage() {

  for (int p = 0; p < 15; p++) {
    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = map(analogRead(rotaryPin), 0, 4095, 0, 127);
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;

    // if we're at the end of the array...
    if (readIndex >= numReadings) {
      // ...wrap around to the beginning:
      readIndex = 0;
    }

    // calculate the average:
    average1 = total / numReadings;
    delay(1);  // delay in between reads for stability
  }
}

void doMIDI() {
  if (button1.rose()) {
    sendNoteOn(buttonNotes[0]);
    buttonPlayed[0] = buttonNotes[0];
  } else if (button1.fell()) {
    sendNoteOff(buttonPlayed[0]);
  }

  if (button2.rose()) {
    sendNoteOn(buttonNotes[1]);
    buttonPlayed[1] = buttonNotes[1];
  } else if (button2.fell()) {
    sendNoteOff(buttonPlayed[1]);
  }

  if (button3.rose()) {
    sendNoteOn(buttonNotes[2]);
    buttonPlayed[2] = buttonNotes[2];
  } else if (button3.fell()) {
    sendNoteOff(buttonPlayed[2]);
  }

  if (button4.rose()) {
    sendNoteOn(buttonNotes[3]);
    buttonPlayed[3] = buttonNotes[3];
  } else if (button4.fell()) {
    sendNoteOff(buttonPlayed[3]);
  }

  if (button5.rose()) {
    sendNoteOn(buttonNotes[4]);
    buttonPlayed[4] = buttonNotes[4];
  } else if (button5.fell()) {
    sendNoteOff(buttonPlayed[4]);
  }

  if (button6.rose()) {
    sendNoteOn(buttonNotes[5]);
    buttonPlayed[5] = buttonNotes[5];
  } else if (button6.fell()) {
    sendNoteOff(buttonPlayed[5]);
  }

  if (button7.rose()) {
    sendNoteOn(buttonNotes[6]);
    buttonPlayed[6] = buttonNotes[6];
  } else if (button7.fell()) {
    sendNoteOff(buttonPlayed[6]);
  }

  if (button8.rose()) {
    sendNoteOn(buttonNotes[7]);
    buttonPlayed[7] = buttonNotes[7];
  } else if (button8.fell()) {
    sendNoteOff(buttonPlayed[7]);
  }

  if (button9.rose()) {
    sendNoteOn(buttonNotes[8]);
    buttonPlayed[8] = buttonNotes[8];
  } else if (button9.fell()) {
    sendNoteOff(buttonPlayed[8]);
  }

  if (button10.rose()) {
    sendNoteOn(buttonNotes[9]);
    buttonPlayed[9] = buttonNotes[9];
  } else if (button10.fell()) {
    sendNoteOff(buttonPlayed[9]);
  }

  if (button11.rose()) {
    sendNoteOn(buttonNotes[10]);
    buttonPlayed[10] = buttonNotes[10];
  } else if (button11.fell()) {
    sendNoteOff(buttonPlayed[10]);
  }

  if (button12.rose()) {
    sendNoteOn(buttonNotes[11]);
    buttonPlayed[11] = buttonNotes[11];
  } else if (button12.fell()) {
    sendNoteOff(buttonPlayed[11]);
  }
}

void setNotes() {
  buttonNotes[0] = valRoot + ((faderValue)*12);
  for (int i = 1; i < 12; i++) {
    buttonNotes[i] = buttonNotes[i - 1] + noteInterval[i - 1];
  }
}

int buttonChoice() {
  updateButtons();

  if (button1.released()) return 0;
  else if (button2.released()) return 1;
  else if (button3.released()) return 2;
  else if (button4.released()) return 3;
  else if (button5.released()) return 4;
  else if (button6.released()) return 5;
  else if (button7.released()) return 6;
  else if (button8.released()) return 7;
  else if (button9.released()) return 8;
  else if (button10.released()) return 9;
  else if (button11.released()) return 10;
  else if (button12.released()) return 11;
  else return -1;

  delay(100);
}

void sendNoteOn(int note) {
  if ((valInput == 1) || (valInput == 2)) {
    midiPacket[2] = midiChan + 144;
    midiPacket[3] = note;
    midiPacket[4] = velocityValue;
    pCharacteristic->setValue(midiPacket, 5);
    pCharacteristic->notify();
  }
#if ENABLE_TRS
  if ((valInput == 0) || (valInput == 2)) DIN_MIDI.sendNoteOn(note, velocityValue, midiChan);
#endif
  delay(1);
}

void sendNoteOff(int note) {
  if ((valInput == 1) || (valInput == 2)) {
    midiPacket[2] = midiChan + 128;
    midiPacket[3] = note;
    midiPacket[4] = 0;
    pCharacteristic->setValue(midiPacket, 5);
    pCharacteristic->notify();
  }

#if ENABLE_TRS
  if ((valInput == 0) || (valInput == 2)) DIN_MIDI.sendNoteOff(note, 0, midiChan);
#endif
  delay(1);
}

void sendCC(int CC, int value) {
  if ((valInput == 1) || (valInput == 2)) {
    midiPacket[2] = midiChan + 176;
    midiPacket[3] = CC;
    midiPacket[4] = value;
    pCharacteristic->setValue(midiPacket, 5);
    pCharacteristic->notify();
  }
#if ENABLE_TRS
  if ((valInput == 0) || (valInput == 2)) DIN_MIDI.sendControlChange(CC, value, midiChan);
#endif
  delay(1);
}

void savePreset() {  // save preset to one of 8 preset slots
  ledTimer = millis();
  int buttonNum = -1;
  bool select = false;
  delay(50);
  flashLEDs(7);

  while (select == false) {
    if (!(ledState) && (millis() > (ledTimer + 1500))) {
      digitalWrite(led_Green, HIGH);
      ledTimer = millis();
      ledState = true;
    } else if ((ledState) && (millis() > (ledTimer + 750))) {
      digitalWrite(led_Green, LOW);
      ledTimer = millis();
      ledState = false;
    }
    buttonNum = buttonChoice();
    switch (buttonNum) {
      case 0 ... 7:  // select between presets 0 through 7
        storeEEPROM(buttonNum);
        flashLEDs((buttonNum + 1) * 2);
        select = true;
        break;
      case 9 ... 11:  // press bottom row to exit and not save
        select = true;
        break;
    }
  }
}

void storeEEPROM(int presetNum) {  // write current settings to EEPROM
  EEPROM.write(baseEEPROM + (presetNum * 10) + 0, valInput);
  EEPROM.write(baseEEPROM + (presetNum * 10) + 1, midiChan);
  EEPROM.write(baseEEPROM + (presetNum * 10) + 2, valScale);
  EEPROM.write(baseEEPROM + (presetNum * 10) + 3, valRoot);
  EEPROM.write(baseEEPROM + (presetNum * 10) + 4, knobFunction);

  EEPROM.commit();
}

void recallEEPROM(int presetNum) {  // recall settings from EEPROM
  valInput = EEPROM.read(baseEEPROM + (presetNum * 10) + 0);
  if (!((valInput > -1) || (valInput < 3))) valInput = DEFAULTINPUT;  // validate input is between 0 and 2
  midiChan = EEPROM.read(baseEEPROM + (presetNum * 10) + 1);
  if (!((midiChan > -1) || (midiChan < 12))) midiChan = DEFAULTCHAN;  // validate channel is between 0 and 11
  valScale = EEPROM.read(baseEEPROM + (presetNum * 10) + 2);
  if (!((valScale > -1) || (valScale < 12))) valScale = DEFAULTSCALE;  // validate scale is between 0 and 11
  valRoot = EEPROM.read(baseEEPROM + (presetNum * 10) + 3);
  if (!((valRoot > -1) || (valRoot < 12))) valRoot = DEFAULTROOT;  // validate root is between 0 and 11
  knobFunction = EEPROM.read(baseEEPROM + (presetNum * 10) + 4);
  if (!((knobFunction > -1) || (knobFunction < 4))) knobFunction = DEFAULTKNOB;  // validate knob function is between 0 and 3

  setupScale(true);  // set note values based on scale
}
