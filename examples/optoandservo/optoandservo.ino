#include <Servo.h>

Servo fuelgauge;  // create servo object to control a servo
Servo tempgauge;
Servo oilgauge;

int fuelsender = 2;  // analog pin used to connect the potentiometer
int tempsender = 3;
int oilsender = 6;

int fuelval;    // variable to read the value from the analog pin
int tempval;
int oilval;

int opto = 4;
int ignition = 7;

void setup() {
  fuelgauge.attach(6);  // attaches the fuel gauge servo to pin 6  
  tempgauge.attach(5);  // attaches the temp gauge servo to pin 5  
  oilgauge.attach(3);   // attaches the oil gauge servo to pin 3  
  pinMode(opto, OUTPUT);
  pinMode(ignition, INPUT);
}

void loop() {
  fuelval = analogRead(fuelsender);            // reads the value of the potentiometer (value between 0 and 1023)
  fuelval = map(fuelval, 0, 1023, 0, 180);     // scale it to use it with the servo (value between 0 and 180)
  fuelgauge.write(fuelval);                  // sets the servo position according to the scaled value

  tempval = analogRead(tempsender);            // reads the value of the potentiometer (value between 0 and 1023)
  tempval = map(tempval, 0, 1023, 0, 180);     // scale it to use it with the servo (value between 0 and 180)
  tempgauge.write(tempval);                  // sets the servo position according to the scaled value

  oilval = analogRead(oilsender);            // reads the value of the potentiometer (value between 0 and 1023)
  oilval = map(oilval, 0, 1023, 0, 180);     // scale it to use it with the servo (value between 0 and 180)
  oilgauge.write(oilval);                  // sets the servo position according to the scaled value

  if (digitalRead(ignition) == HIGH)
  {
    digitalWrite(opto, LOW);
  }
 else digitalWrite(opto, HIGH);
}
