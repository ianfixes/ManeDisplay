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

// This just provides all the library functions the DashState.h file will need.
// Doing it this way allows us to swap in different functions for unit testing.
DashSupport ds = {
  pinMode,
  analogRead,
  digitalRead,
  digitalWrite,
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
  dash.setup();
  Wire.begin(SLAVE_I2C_ADDRESS);      // Start the I2C Bus as Slave on address
  Wire.onReceive(receiveDashMessage); // Attach a function to trigger when something is received.
}

void loop() {
  const unsigned long currentMillis = millis();
  SlaveState state(digitalRead, analogRead);
  dash.setState(state);
  dash.apply(currentMillis);
}
