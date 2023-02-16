/*
*/
#include <Wire.h>
#include <FastLED.h>
#include <SlaveProperties.h>
#include <DashMessage.h>
#include <DashState.h>

DashSupport ds = {
  pinMode,
  analogRead,
  digitalRead,
  digitalWrite,
  &FastLED
};
DashState dash(ds);

void receiveDashMessage(int bytes) {
  bytes; // suppress a compiler warning; we won't use this
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
