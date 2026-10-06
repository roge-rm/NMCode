#include <Arduino.h>
#include <MIDI.h>

#include "config.h"
#include "globals.h"
#include "functions.h"

#if ENABLE_BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"
#endif

#if ENABLE_TRS
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, DIN_MIDI);
static bool trsStarted = false;
#endif

static int buttonPlayed[4][12]; // what notes were played by each button last (in case the octave/scale is changed while a note is being played; to prevent hung notes), -1 = none

#if ENABLE_BLE
static BLECharacteristic *pCharacteristic = nullptr; // stays null until BLE output is first enabled

static uint8_t midiPacket[] = {
    0x80, // header
    0x80, // timestamp, not implemented
    0x00, // status
    0x3c, // 0x3c == 60 == middle c
    0x00  // velocity
};

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    pServer->getAdvertising()->start(); // keep advertising so the device can reconnect
  }
};
#endif

static bool useTRS() { return (valOutput == 0) || (valOutput == 2); }
static bool useBLE() { return (valOutput == 1) || (valOutput == 2); }

void initOutputs()
{ // start whichever outputs the current settings need and haven't been started yet - safe to call
  // again after a setup/preset change at runtime, since BLE can't be torn down once started
#if ENABLE_TRS
  if (useTRS() && !trsStarted)
  {
    DIN_MIDI.begin(MIDI_CHANNEL_OMNI);
    trsStarted = true;
  }
#endif

#if ENABLE_BLE
  if (useBLE() && (pCharacteristic == nullptr))
  {
    BLEDevice::init(BLENAME);

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
#endif

  for (int i = 0; i < 12; i++)
    for (int n = 0; n < 4; n++)
      buttonPlayed[n][i] = -1;
}

void readMIDI()
{
#if ENABLE_TRS
  if (trsStarted)
    DIN_MIDI.read();
#endif
}

static void sendBLE(uint8_t status, uint8_t data1, uint8_t data2)
{
#if ENABLE_BLE
  if (!useBLE() || (pCharacteristic == nullptr) || !deviceConnected)
    return;
  midiPacket[2] = status | ((midiChan - 1) & 0x0F); // midiChan is 1-16, the status byte's channel nibble is 0-15
  midiPacket[3] = data1;
  midiPacket[4] = data2;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
#endif
}

static bool sendNoteOn(int note)
{ // returns false if the note falls outside the MIDI range and was skipped
  if ((note < 0) || (note > 127))
    return false;
  sendBLE(0x90, note, velocityValue);
#if ENABLE_TRS
  if (useTRS())
    DIN_MIDI.sendNoteOn(note, velocityValue, midiChan);
#endif
  delay(1);
  return true;
}

static void sendNoteOff(int note)
{
  if ((note < 0) || (note > 127))
    return;
  sendBLE(0x80, note, 0);
#if ENABLE_TRS
  if (useTRS())
    DIN_MIDI.sendNoteOff(note, 0, midiChan);
#endif
  delay(1);
}

void sendCC(int CC, int value)
{
  sendBLE(0xB0, CC, value);
#if ENABLE_TRS
  if (useTRS())
    DIN_MIDI.sendControlChange(CC, value, midiChan);
#endif
  delay(1);
}

void midiON(int bNum)
{
  buttonIsOn[bNum] = true;
  buttonPlayed[0][bNum] = sendNoteOn(buttonNotes[bNum]) ? buttonNotes[bNum] : -1;
  buttonPlayed[1][bNum] = -1;
  buttonPlayed[2][bNum] = -1;
  buttonPlayed[3][bNum] = -1;
}

// plays the diatonic chord built on the given scale degree (0-5, i.e. degrees 1-6), stacked in
// thirds directly off the scale ladder in buttonNotes[] so it automatically follows whatever
// scale/root/octave is active. octaveShift (in semitones, e.g. +/-12 per octave) transposes the
// whole chord, without needing a second scale ladder.
void midiONChord(int bNum, int degree, int octaveShift)
{
  buttonIsOn[bNum] = true;
  int notes[4];
  notes[0] = buttonNotes[degree] + octaveShift;     // root
  notes[1] = buttonNotes[degree + 2] + octaveShift; // third
  notes[2] = buttonNotes[degree + 4] + octaveShift; // fifth
  notes[3] = (chordVoicing == 1) ? buttonNotes[degree + 6] + octaveShift : -1; // seventh

  for (int n = 0; n < 4; n++)
    buttonPlayed[n][bNum] = sendNoteOn(notes[n]) ? notes[n] : -1;
}

void midiOFF(int bNum)
{
  buttonIsOn[bNum] = false;
  for (int n = 0; n < 4; n++)
  {
    sendNoteOff(buttonPlayed[n][bNum]); // out of range (-1) entries are skipped
    buttonPlayed[n][bNum] = -1;
  }
}
