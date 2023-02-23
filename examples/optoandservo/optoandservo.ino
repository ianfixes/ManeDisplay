#include <CalibratedServo.h>
#include <SlaveProperties.h>

const Range inputRange { 0, 1023 };
const Range outputRange { 0, 180 };

// the pin, input min/max and output min/max are now part of the initialization
CalibratedServo fuelgauge(SlavePin::Values::fuelServo, inputRange, outputRange);
CalibratedServo tempgauge(SlavePin::Values::tempServo, inputRange, outputRange);
CalibratedServo oilgauge( SlavePin::Values::oilServo,  inputRange, outputRange);

void setup() {
  fuelgauge.setup();
  tempgauge.setup();
  oilgauge.setup();

  pinMode(SlavePin::Values::optoCoupler, OUTPUT);
  pinMode(SlavePin::Values::ignitionInput, INPUT);
}

void loop() {
  fuelgauge.write(analogRead(SlavePin::Values::fuelInput));
  tempgauge.write(analogRead(SlavePin::Values::temperatureInput));
  oilgauge.write(analogRead(SlavePin::Values::oilInput));

  // set the optocoupler to be the opposite of the ignition state
  const int useOpto = digitalRead(SlavePin::Values::ignitionInput) == LOW;
  digitalWrite(SlavePin::Values::optoCoupler, useOpto ? HIGH : LOW);
}
