#pragma once
#include <Arduino.h>
#include <Servo.h>
#include "DashMessage.h"

/**
 * This file defines some of the properties of the slave board
 *
 */

// digital pin assignments for the slave
namespace SlavePin {
  enum Values {
    ignitionInput      = A0, // input signal during ignition to light up the dash
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

// This struct is responsible for all of the reading and bookkeeping of
// the information that the slave board can collect
typedef struct SlaveState {
  bool scrollCAN;
  bool backlightDim;
  bool tachometerCritical;
  bool tachometerWarning;
  bool ignition;

  int fuelLevel;
  int temperatureLevel;
  int oilPressureLevel;

  DashMessage masterMessage;

  // get a single bit from the master signals
  inline void setMasterSignals(DashMessage const &m) {
    masterMessage = m;
  }

  // get a single bit from the master signals
  inline void setMasterSignalsFromWire(TwoWire &wire) {
    masterMessage.setFromWire(wire);
  }

  // get a single bit from the master signals
  inline bool getMasterSignal(MasterSignal::Values position) const {
    return masterMessage.getBit(position);
  }

  // Best if we keep the necessary setup for all the pins in this class,
  // since it needs to agree with the code that reads from those pins
  static void setup(void (*myPinMode)(pin_size_t, int)) {
    myPinMode(SlavePin::Values::scrollCAN,          INPUT);
    myPinMode(SlavePin::Values::backlightDim,       INPUT);
    myPinMode(SlavePin::Values::tachometerCritical, INPUT);
    myPinMode(SlavePin::Values::tachometerWarning,  INPUT);
    myPinMode(SlavePin::Values::fuelInput,          INPUT);
    myPinMode(SlavePin::Values::temperatureInput,   INPUT);
    myPinMode(SlavePin::Values::oilInput,           INPUT);
    myPinMode(SlavePin::Values::ignitionInput,      INPUT);
  }

  // read payload from digital input pins into the fields of this struct
  void setFromPins(PinStatus (*myDigitalRead)(pin_size_t), int (*myAnalogRead)(pin_size_t)) {
    scrollCAN          = myDigitalRead(SlavePin::Values::scrollCAN);
    backlightDim       = myDigitalRead(SlavePin::Values::backlightDim);
    tachometerCritical = myDigitalRead(SlavePin::Values::tachometerCritical);
    tachometerWarning  = myDigitalRead(SlavePin::Values::tachometerWarning);
    ignition           = myDigitalRead(SlavePin::Values::ignitionInput);

    fuelLevel        = myAnalogRead(SlavePin::Values::fuelInput);
    temperatureLevel = myAnalogRead(SlavePin::Values::temperatureInput);
    oilPressureLevel = myAnalogRead(SlavePin::Values::oilInput);
  }

  // make a binary representation of what's in the message
  String toString() const {
    String ret = "";
    ret.concat(ignition           ? "I" : "i");
    ret.concat(backlightDim       ? "b" : "B");
    ret.concat(scrollCAN          ? "S" : "s");
    ret.concat(tachometerCritical ? "C" : (tachometerWarning ? "W" : "_"));
    ret.concat("\t");

    char buf[16];
    sprintf(buf, "%04d %04d %04d ", fuelLevel, temperatureLevel, oilPressureLevel);
    ret.concat(buf);
    ret.concat(masterMessage.binaryString());
    return ret;
  }

  // shortcut: construct directly from the pin states
  SlaveState(PinStatus (*myDigitalRead)(pin_size_t), int (*myAnalogRead)(pin_size_t)) {
    setFromPins(myDigitalRead, myAnalogRead);
  }

  // default constructor: anything goes. need to define this because
  // the previous constructor definition prevents the implicit default
  SlaveState() { }

} SlaveState;
