#pragma once

/**
 * This file defines some of the properties of the slave board
 *
 */

// digital pin assignments for the slave
namespace SlavePin {
  enum Values {
    scrollCAN          = 3, // scroll the CAN display
    backlightDim       = 5,
    tachometerCritical = 7,
    tachometerWarning  = 9,
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
