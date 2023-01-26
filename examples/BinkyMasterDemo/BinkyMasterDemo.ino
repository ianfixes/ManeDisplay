/**
 * Troubleshooting / debugging program for the Binky instrument panel.
 *
 * - Send I2C requests to the slave, timed to a morse code "BOM" pattern
 *
 * Based on https://create.arduino.cc/projecthub/PIYUSH_K_SINGH/master-slave-i2c-connection-f1aa53
 */

#include <Wire.h>
#include <MasterProperties.h>
#include <DashMessage.h>

// top-level settings
const int morseWPM        = 10;

// settings calculated from top-level settings
const int lenDit            = int(60.0 * 1000 / (50.0 * morseWPM)); // https://morsecode.world/international/timing.html in milliseconds
const int lenDah            = lenDit * 3;
const int lenSpaceIntraChar = lenDit;
const int lenSpaceInterChar = (lenDit * 3) - lenSpaceIntraChar;
const int lenSpaceInterWord = (lenDit * 7) - lenSpaceInterChar;

// variables
bool ledState = HIGH;

// send our desired state to the slave device
void triggerSlave(int state) {
  DashMessage d;
  d.setBit(MasterSignal::Values::led23to100pctAmber, state);
  d.send(Wire, SLAVE_I2C_ADDRESS);
}

// to send a morse code character, turn on for a desired period of time then off for standard time
void sendPulse(int len) {
  triggerSlave(1);
  delay(len);
  triggerSlave(0);
  delay(lenSpaceIntraChar);
}

// one-liners for the morse basics
void dit() { sendPulse(lenDit); }
void dah() { sendPulse(lenDah); }

// definitions of the letters we need
void sendB() {
  dah(); dit(); dit(); dit();
  delay(lenSpaceInterChar);
}

void sendO() {
  dah(); dah(); dah();
  delay(lenSpaceInterChar);
}

void sendM() {
  dah(); dah();
  delay(lenSpaceInterChar);
}

void nextWord() {
  delay(lenSpaceInterWord);
}

// provide some indication that things are cycling along
void flipLed() {
  digitalWrite(LED_BUILTIN, ledState);
  ledState = (ledState == LOW) ? HIGH : LOW;
}


/////////////////////////////////// arduino functions

void setup() {
  Wire.begin(); // I2C bus master -- no ID
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  flipLed();

  sendB();
  sendO();
  sendM();
  nextWord();
}
