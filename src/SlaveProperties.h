#pragma once
#include <Arduino.h>

/**
 * This file defines some of the properties of the slave board
 *
 */

// digital pin assignments for the slave
namespace SlavePin {
  enum Values {
    idiotLights        = A0,
    fuelWarning        = A1,
    fuelInput          = A2, // analog in fuel level
    temperatureInput   = A3, // analog in temp gauge
    oilInput           = A6, // analog in oil pressure
    scrollCAN          = A7, // odo trip switch
    oilServo           = 3,
    optoCoupler        = 4,  // alternate power source enable
    tempServo          = 5,
    fuelServo          = 6,
    backlightDim       = 8,
    tachometerCritical = 9,
    tachometerWarning  = 10,
    ledStrip           = 11,
    ledBuiltin         = LED_BUILTIN,
  };
}

typedef struct SlaveState {
  bool scrollCAN;
  bool backlightDim;
  bool tachometerCritical;
  bool tachometerWarning;

  // read payload from digital input pins
  void setFromPins(int (*myDigitalRead)(unsigned char)) {
    scrollCAN          = myDigitalRead(SlavePin::Values::scrollCAN);
    backlightDim       = myDigitalRead(SlavePin::Values::backlightDim);
    tachometerCritical = myDigitalRead(SlavePin::Values::tachometerCritical);
    tachometerWarning  = myDigitalRead(SlavePin::Values::tachometerWarning);
  }

} SlaveState;
