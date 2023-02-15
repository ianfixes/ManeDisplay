/**
 * Project Binky master dashboard program -- it just sends pin values to the slave board
 */

#include <Wire.h>
#include <DashMessage.h>

void setup() {
  Wire.begin(); // I2C bus master -- no ID
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  DashMessage d(digitalRead); // create a message from the current state of pins
  d.send(Wire, SLAVE_I2C_ADDRESS);

  delay(20); // not sure how often we need to send updates, but ~50 times per second is probably OK
}
