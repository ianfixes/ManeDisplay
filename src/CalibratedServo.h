#pragma once
#include <Servo.h>

// a calibrated servo defines its input (signal) and output (position) ranges
// and calculates them behind the scenes, so our code can be more concise
// at the point where the servos are used
typedef struct CalibratedServo {
  Servo servo;                  // underlying servo code
  const unsigned char pin;      // pin we want to use
  const unsigned int inputMin;  // minimum expected input level
  const unsigned int inputMax;  // maximum expected input level (clamped to 1023)
  const unsigned int outputMin; // minimum allowed output level
  const unsigned int outputMax; // maximum allowed output level (clamped to 180)

  void setup() {
    servo.attach(pin);
  }

  // constrain the input and output as well as mapping it
  void write(int inputPosition) {
    const int outputPosition = map(
      inputPosition,
      max(0,    inputMin),
      min(1023, inputMax),
      max(0,    outputMin),
      min(180,  outputMax)
    );
    servo.write(outputPosition);
  }

  // the constructor is just delegating all the values to the member constructors
  CalibratedServo(
    unsigned char servoPin,
    unsigned int inMin,
    unsigned int inMax,
    unsigned int outMin,
    unsigned int outMax
  ): pin(servoPin), inputMin(inMin), inputMax(inMax), outputMin(outMin), outputMax(outMax) {}
} CalibratedServo;
