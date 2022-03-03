/*
   NMCode by this.is.NOISE inc.

   https://github.com/thisisnoiseinc/NMCode

   Built upon:

    "BLE_MIDI Example by neilbags
    https://github.com/neilbags/arduino-esp32-BLE-MIDI

    Based on BLE_notify example by Evandro Copercini."

    -----
    RM.edit by rm, for my own Sunvox-related purposes.
    This software 7 different scales (or no scale) as well as choosing the root note of your choice.
    See https://github.com/hunked/NMCode for more information.
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

const int numButtons = 12; // number of buttons

// Pin assignments

int faderPin = 36; // Slider
int rotaryPin = 39; // Rotary Knob
int led_Blue = 14; // BLE LED
int led_Green = 4; // CHANNEL LED
int buttonPins[numButtons] = {16, 17, 18, 21, 19, 25, 22, 23, 27, 26, 35, 34};

// Bool operators

bool deviceConnected = false;
bool selectChan = false;
bool selectRoot = false; // run loop to select root note
bool selectScale = false; // run loop to select scale
bool selectKnob = false; // run loop to select knob function
bool stateChange = false; // flag set to run note reassignment

int velocityValue = 100; // MIDI velocity value

int knobFunction = 0; // change knob function: 0 = velocity, 1 = modulation, 2 = pan, 3 = expression

int midiChan = -1; // MIDI channel to send data on
int noteRoot = 0; // default middle C root note

int noteInterval[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // number of semitones each note is apart, default 1 for no scale
int scaleMajor[11] = {2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2};
int scaleNatMinor[11] = {2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2};
int scaleHarMinor[11] = {2, 1, 2, 2, 1, 3, 1, 2, 1, 2, 2};
int scalePentMajor[11] = {2, 2, 3, 2, 3, 2, 2, 3, 2, 3, 2};
int scalePentMinor[11] = {3, 2, 2, 3, 2, 3, 2, 2, 3, 2, 3};
int scaleWholeTone[11] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
int scaleBlues[11] = {3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 3};

int buttonNotes[12]; // currently assigned button note
int buttonPlayed[12]; // what note was played by each button last (in case the page of notes is changed while a note is being played; to prevent hung notes)

// Fader/rotary variables
int faderValue = 4;

const int numReadings = 15;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average1 = 0; // average current state
int lastaverage1 = 0; // average previous state

BLECharacteristic *pCharacteristic;

uint8_t midiPacket[] = {
  0x80,  // header
  0x80,  // timestamp, not implemented
  0x00,  // status
  0x3c,  // 0x3c == 60 == middle c
  0x00   // velocity
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
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
  int buttonNum = -1; // used to return number of button pressed

  //Serial.begin(115200);

  BLEDevice::init("NMSVE.rm");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID(CHARACTERISTIC_UUID),
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();

  // Button setup
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

  // Button debounce interval in ms
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

  // Needed for analog debounce?
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  updateButtons();

  // Set up LED pins
  pinMode (led_Blue, OUTPUT);
  pinMode (led_Green, OUTPUT);

  delay(500);

  while (selectChan == false) { // select MIDI channel
    digitalWrite(led_Green, HIGH);
    buttonNum = buttonChoice();
    if (buttonNum > -1) {
      midiChan = buttonNum;
      selectChan = true;
      buttonNum = -1;

      delay(50);
      updateButtons();
    }
  }

  flashLEDs(1);

  while (selectScale == false) { // select scale

    digitalWrite(led_Green, HIGH);
    buttonNum = buttonChoice();

    if ((buttonNum > -1) && (buttonNum < 9)) {

      switch (buttonNum) {
        case 0: // major scale
          memcpy(noteInterval, scaleMajor, sizeof noteInterval);
          break;
        case 1: // natural minor
          memcpy(noteInterval, scaleNatMinor, sizeof noteInterval);
          break;
        case 2: // harmonic minor
          memcpy(noteInterval, scaleHarMinor, sizeof noteInterval);
          break;
        case 3: // no scale
          break;
        case 4: // pentatonic major
          memcpy(noteInterval, scalePentMajor, sizeof noteInterval);
          break;
        case 5: // pentatonic minor
          memcpy(noteInterval, scalePentMinor, sizeof noteInterval);
          break;
        case 6: // whole tone
          memcpy(noteInterval, scaleWholeTone, sizeof noteInterval);
          break;
        case 7: // blues
          memcpy(noteInterval, scaleBlues, sizeof noteInterval);
          break;
      }
      selectScale = true;

      delay(50);
      updateButtons();

      buttonNum = -1;
    }
  }

  flashLEDs(2);

  while (selectRoot == false) { // select root note

    digitalWrite(led_Green, HIGH);
    buttonNum = buttonChoice();
    if ((buttonNum > -1) && (buttonNum < 12)) {
      noteRoot = buttonNum;
      selectRoot = true;
      delay(50);
      updateButtons();
      buttonNum = -1;
    }
  }

  flashLEDs(3);

  while (selectKnob == false) { // select root note

    digitalWrite(led_Green, HIGH);
    buttonNum = buttonChoice();
    if ((buttonNum > -1) && (buttonNum < 4)) {
      knobFunction = buttonNum;
      selectKnob = true;
      delay(50);
      updateButtons();
      buttonNum = -1;
    }
  }

  flashLEDs(4);
  updatePots(); // get initial pot locations (used for setting octave/velocity)
  setNotes(); // set initial note values
}

void loop() {
  // Ensure device is connected to BLE
  if (deviceConnected == false) {
    digitalWrite(led_Blue, HIGH);
    delay(1000);
    digitalWrite(led_Blue, LOW);
    delay(1000);
  }

  // Enter Default Mode
  else {
    digitalWrite(led_Blue, HIGH);
    updatePots(); // Check for changes to pot/fader

    if (stateChange) { // Change octave if needed
      setNotes();
      stateChange = false;

      if (faderValue == 0 || faderValue == 2 || faderValue == 4 || faderValue == 6 || faderValue == 8) {
        digitalWrite(led_Green, LOW);
      }
      else {
        digitalWrite(led_Green, HIGH);
      }
    }

    updateButtons(); // Check for button presses
    doMIDI(); // Send any required MIDI messages
  }
}

void flashLEDs(int flashes) {
  for (int i = 0; i < flashes; i++) {
    digitalWrite(led_Green, LOW);
    digitalWrite(led_Blue, HIGH);
    delay(50);
    digitalWrite(led_Blue, LOW);
    delay(100);
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
      case 0: // velocity
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
    delay(1);        // delay in between reads for stability
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
  buttonNotes[0] = noteRoot + ((faderValue) * 12);
  for (int i = 1; i < 12; i++) {
    buttonNotes[i] = buttonNotes[i - 1] + noteInterval[i - 1];
  }
}

int buttonChoice() {
  updateButtons();

  if (button1.rose()) return 0;
  else if (button2.rose()) return 1;
  else if (button3.rose()) return 2;
  else if (button4.rose()) return 3;
  else if (button5.rose()) return 4;
  else if (button6.rose()) return 5;
  else if (button7.rose()) return 6;
  else if (button8.rose()) return 7;
  else if (button9.rose()) return 8;
  else if (button10.rose()) return 9;
  else if (button11.rose()) return 10;
  else if (button12.rose()) return 11;
  else return -1;

  delay(100);
}

void sendNoteOn(int note) {
  midiPacket[2] = midiChan + 144;
  midiPacket[3] = note;
  midiPacket[4] = velocityValue;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
  delay(1);
}

void sendNoteOff(int note) {
  midiPacket[2] = midiChan + 128;
  midiPacket[3] = note;
  midiPacket[4] = 0;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
  delay(1);
}

void sendCC(int CC, int value) {
  midiPacket[2] = midiChan + 176;
  midiPacket[3] = CC;
  midiPacket[4] = value;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
  delay(1);
}
