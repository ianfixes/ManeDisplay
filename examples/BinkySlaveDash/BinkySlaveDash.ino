/*
 * The binky dashboard slave board is responsible for receiving messages from the
 * master and reading pins state.  From there, everything is delegated to the DashState.h
 * library functions
 *
 * Messages from the wire are passed to the dash asynchronously.
 * Pin states are passed synchronously.
 * That information is then applied to the hardware.
 */
#include <Wire.h>
#include <FastLED.h>
#include <SlaveProperties.h>
#include <DashMessage.h>
#include <DashState.h>

// work around a VERY ANNOYING PROBLEM with how arduino defines its internal functions
#ifndef pin_size_t
  void myPinMode(unsigned char pin, unsigned char mode) {
    pinMode(pin, mode);
  }

  int myAnalogRead(unsigned char pin) {
    return analogRead(pin);
  }

  int myDigitalRead(unsigned char pin) {
    return digitalRead(pin);
  }

  void myDigitalWrite(unsigned char pin, int val) {
    digitalWrite(pin, val);
  }
#else
  void myPinMode(pin_size_t pin, pin_size_t mode) {
    pinMode(pin, mode);
  }

  int myAnalogRead(pin_size_t pin) {
    return analogRead(pin);
  }

  int myDigitalRead(pin_size_t pin) {
    return digitalRead(pin);
  }

  void myDigitalWrite(pin_size_t pin, int val) {
    digitalWrite(pin, val);
  }
#endif



// This just provides all the library functions the DashState.h file will need.
// Doing it this way allows us to swap in different functions for unit testing.
DashSupport ds = {
  myPinMode,
  myAnalogRead,
  myDigitalRead,
  myDigitalWrite,
  &FastLED
};

DashState dash(ds);

void receiveDashMessage(int /* bytes */) {
  DashMessage dm;
  // consume all available messages
  while (Wire.available() >= (int)WIRE_PROTOCOL_MESSAGE_LENGTH) {
    // decode a message.  if it's valid, pass it along to the dash
    dm.setFromWire(Wire);
    if (!dm.isError()) {
      dash.setMessage(dm);
    }
  }
}

void setup() {
  // turn off any LEDs we aren't using
  struct CRGB leds[64];
  FastLED.addLeds<LED_TYPE, SlavePin::Values::ledStrip, COLOR_ORDER>(leds, 64).setCorrection(TypicalLEDStrip);
  for (int i = 0; i < 64; ++i) leds[i] = CRGB::HTMLColorCode(CRGB::Black);
  FastLED.show();

  // serial debugging
  // Serial.begin(1000000);

  dash.setup();

  Wire.begin(SLAVE_I2C_ADDRESS);      // Start the I2C Bus as Slave on address
  Wire.onReceive(receiveDashMessage); // Attach a function to trigger when something is received.
}

void loop() {
  const unsigned long currentMillis = millis();
  dash.setSlaveState(myDigitalRead, myAnalogRead);

  dash.apply(currentMillis);
  // Serial.println(dash.lastStateString(currentMillis));
}
