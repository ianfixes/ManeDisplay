#include <Servo.h>
#include <SlaveProperties.h>

Servo fuelgauge;  // create servo object to control a servo
Servo tempgauge;
Servo oilgauge;

void setup() {
  fuelgauge.attach(SlavePin::Values::fuelServo);  // attaches the fuel gauge servo to pin 6
  tempgauge.attach(SlavePin::Values::tempServo);  // attaches the temp gauge servo to pin 5
  oilgauge.attach( SlavePin::Values::oilServo);   // attaches the oil gauge servo to pin 3

  pinMode(SlavePin::Values::optoCoupler, OUTPUT);
  pinMode(SlavePin::Values::ignitionInput, INPUT);
}

void loop() {
  const int fuelSignal = analogRead(SlavePin::Values::fuelInput);   // reads the value of the potentiometer (value between 0 and 1023)
  const int fuelPosition = map(fuelSignal, 0, 1023, 0, 180);        // scale it to use it with the servo (value between 0 and 180)
  fuelgauge.write(fuelPosition);                            // sets the servo position according to the scaled value

  const int tempSignal = analogRead(SlavePin::Values::temperatureInput);  // reads the value of the potentiometer (value between 0 and 1023)
  const int tempPosition = map(tempSignal, 0, 1023, 0, 180);      // scale it to use it with the servo (value between 0 and 180)
  tempgauge.write(tempPosition);                                  // sets the servo position according to the scaled value

  const int oilSignal = analogRead(SlavePin::Values::oilInput);  // reads the value of the potentiometer (value between 0 and 1023)
  const int oilPosition = map(oilSignal, 0, 1023, 0, 180);       // scale it to use it with the servo (value between 0 and 180)
  oilgauge.write(oilPosition);                           // sets the servo position according to the scaled value

  // set the optocoupler to be the opposite of the ignition state
  const int useOpto = digitalRead(SlavePin::Values::ignitionInput) == LOW;
  digitalWrite(SlavePin::Values::optoCoupler, useOpto ? HIGH : LOW);
}
