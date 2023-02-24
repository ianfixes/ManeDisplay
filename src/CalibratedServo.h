#pragma once
#include <Servo.h>

// A range defines a lower and upper bound
typedef struct Range {
  unsigned int min;
  unsigned int max;

  unsigned int clamp(unsigned int v) const { return constrain(v, min, max); }
} Range;

// a calibrated servo defines its input (signal) and output (position) ranges
// and calculates them behind the scenes, so our code can be more concise
// at the point where the servos are used
typedef struct CalibratedServo {
  Servo servo;              // underlying servo code
  const unsigned char pin;  // pin we want to use
  const Range inputRange;   // expected input range
  const Range outputRange;  // allowed output range

  void setup() {
    servo.attach(pin);
  }

  // constrain the input and output as well as mapping it
  void write(int inputPosition) {
    const int outputPosition = map(
      inputPosition,
      inputRange.min,
      inputRange.max,
      outputRange.min,
      outputRange.max
    );
    servo.write(outputPosition);
  }

  // the constructor is just delegating all the values to the member constructors
  CalibratedServo(
    unsigned char servoPin,
    Range inRange,
    Range outRange
  ) : pin(servoPin), inputRange(inRange), outputRange(outRange) {}
} CalibratedServo;
