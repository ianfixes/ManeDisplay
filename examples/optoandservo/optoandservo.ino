#include <Servo.h>
#include <SlaveProperties.h>
#include <DashState.h>

// the pin, input min/max and output min/max are now part of the initialization
CalibratedServo fuelgauge(SlavePin::Values::fuelServo, 0, 1023, 0, 180);
CalibratedServo tempgauge(SlavePin::Values::tempServo, 0, 1023, 0, 180);
CalibratedServo oilgauge( SlavePin::Values::oilServo,  0, 1023, 0, 180);

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
