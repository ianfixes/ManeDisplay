#pragma once
#include <Arduino.h>
#include "DashMessage.h"
#include "Debouncer.h"

unsigned int const DEBOUNCE_TIME_MS = 50;
unsigned int const SCROLLCAN_PULSE_TIME = 50; // The duration of the HIGH signal to output when scrolling CAN

// we may define a bunch of rainbow modes, and here is how we keep track of them
typedef struct EffectMode {
  enum Values {
    none    = 0,
    rainbow = 1,
    sparkle = 2,
    shimmer = 3
  };

  Values state;

  EffectMode() { state = none; }
  EffectMode(Values v) { state = v; }
  inline int maxValue() const { return Values::shimmer; }
  inline int isEffect() const { return state != Values::none; }
  inline void next() { state = (state >= maxValue()) ? Values::none : (Values)((int)state + 1); }
  inline void nextEffect() { do { next(); } while (!isEffect()); } // select the next mode, effect modes only
} EffectMode;

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
  bool backlightDim;
  bool tachometerCritical;
  bool tachometerWarning;
  bool ignition;

  int fuelLevel;
  int temperatureLevel;
  int oilPressureLevel;

  DashMessage masterMessage;

  Debouncer CANEvent;
  Debouncer colorEvent;
  Debouncer effectsEvent;     //TODO: mark private
  Debouncer brightnessEvent;

  unsigned long CANPulseBegin; // the time at which a CAN pulse should start

  EffectMode effectmode;

  // set the master signals
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
  static void setup(void (*myPinMode)(uint8_t, uint8_t)) {
    myPinMode(SlavePin::Values::backlightDim,       INPUT);
    myPinMode(SlavePin::Values::tachometerCritical, INPUT);
    myPinMode(SlavePin::Values::tachometerWarning,  INPUT);
    myPinMode(SlavePin::Values::fuelInput,          INPUT);
    myPinMode(SlavePin::Values::temperatureInput,   INPUT);
    myPinMode(SlavePin::Values::oilInput,           INPUT);
    myPinMode(SlavePin::Values::ignitionInput,      INPUT);
  }


#ifdef pin_size_t
  // Best if we keep the necessary setup for all the pins in this class,
  // since it needs to agree with the code that reads from those pins
  static void setup(void (*myPinMode)(pin_size_t, int)) {
    myPinMode(SlavePin::Values::backlightDim,       INPUT);
    myPinMode(SlavePin::Values::tachometerCritical, INPUT);
    myPinMode(SlavePin::Values::tachometerWarning,  INPUT);
    myPinMode(SlavePin::Values::fuelInput,          INPUT);
    myPinMode(SlavePin::Values::temperatureInput,   INPUT);
    myPinMode(SlavePin::Values::oilInput,           INPUT);
    myPinMode(SlavePin::Values::ignitionInput,      INPUT);
  }
#endif

  // read payload from digital input pins into the fields of this struct
  void setFromPins(int (*myDigitalRead)(pin_size_t), int (*myAnalogRead)(pin_size_t)) {
    backlightDim       = myDigitalRead(SlavePin::Values::backlightDim);
    tachometerCritical = myDigitalRead(SlavePin::Values::tachometerCritical);
    tachometerWarning  = myDigitalRead(SlavePin::Values::tachometerWarning);
    ignition           = myDigitalRead(SlavePin::Values::ignitionInput);

    fuelLevel        = myAnalogRead(SlavePin::Values::fuelInput);
    temperatureLevel = myAnalogRead(SlavePin::Values::temperatureInput);
    oilPressureLevel = myAnalogRead(SlavePin::Values::oilInput);
  }

#ifdef PinStatus
  // read payload from digital input pins into the fields of this struct
  void setFromPins(PinStatus (*myDigitalRead)(pin_size_t), int (*myAnalogRead)(pin_size_t)) {
    backlightDim       = myDigitalRead(SlavePin::Values::backlightDim);
    tachometerCritical = myDigitalRead(SlavePin::Values::tachometerCritical);
    tachometerWarning  = myDigitalRead(SlavePin::Values::tachometerWarning);
    ignition           = myDigitalRead(SlavePin::Values::ignitionInput);

    fuelLevel        = myAnalogRead(SlavePin::Values::fuelInput);
    temperatureLevel = myAnalogRead(SlavePin::Values::temperatureInput);
    oilPressureLevel = myAnalogRead(SlavePin::Values::oilInput);
  }
#endif

  void debounce(unsigned long const &millis) {
    // TODO: convert these into eventOf things
    CANEvent.process(millis, masterMessage.getBit(MasterSignal::Values::scrollCAN));
    colorEvent.process(millis, masterMessage.getBit(MasterSignal::Values::scrollPresetColours));
    brightnessEvent.process(millis, masterMessage.getBit(MasterSignal::Values::scrollBrightness));

    if (Debouncer::Event::toHigh == CANEvent.eventOf(millis, masterMessage.getBit(MasterSignal::Values::scrollCAN))) {
      CANPulseBegin = millis;
    }

    if (Debouncer::Event::toHigh == effectsEvent.eventOf(millis, masterMessage.getBit(MasterSignal::Values::scrollRainbowEffects))) {
      effectmode.next();
    }
  }

  // whether the signal to scroll CAN should be high
  bool scrollCANstate(unsigned long const &millis) {
    return SCROLLCAN_PULSE_TIME < millis // don't pulse when the car is first turned on
      && CANPulseBegin <= millis         // do pulse if we should start pulsing
      && millis < (CANPulseBegin + SCROLLCAN_PULSE_TIME); // stop pulse after it expires
  }

  // make a binary representation of what's in the message
  String toString() const {
    String ret = "";
    ret.concat(ignition           ? "I" : "i");
    ret.concat(backlightDim       ? "b" : "B");
    ret.concat(tachometerCritical ? "C" : (tachometerWarning ? "W" : "_"));

    ret.concat(" ");
    if (effectsEvent.last == Debouncer::Event::none) {
      ret.concat("_");
    } else if (effectsEvent.last == Debouncer::Event::toLow) {
      ret.concat("v");
    } else if (effectsEvent.last == Debouncer::Event::toHigh) {
      ret.concat("/");
    }
    ret.concat(" ");
    if (effectmode.state == EffectMode::Values::none) {
      ret.concat("N");
    } else if (effectmode.state == EffectMode::Values::rainbow) {
      ret.concat("R");
    } else if (effectmode.state == EffectMode::Values::sparkle) {
      ret.concat("S");
    }
    ret.concat("\t");

    char buf[16];
    sprintf(buf, "%04d %04d %04d ", fuelLevel, temperatureLevel, oilPressureLevel);
    ret.concat(buf);
    ret.concat(masterMessage.binaryString());
    return ret;
  }

  // default constructor: assign the debounce times. this means we need to define all other
  // constructors and assignment operators
  SlaveState() :
    backlightDim(0),
    tachometerCritical(0),
    tachometerWarning(0),
    ignition(0),
    fuelLevel(0),
    temperatureLevel(0),
    oilPressureLevel(),
    CANEvent(DEBOUNCE_TIME_MS),
    colorEvent(DEBOUNCE_TIME_MS),
    effectsEvent(DEBOUNCE_TIME_MS),
    brightnessEvent(DEBOUNCE_TIME_MS),
    CANPulseBegin(0)
  { }

  // shortcut: construct directly from the pin states
  SlaveState(int (*myDigitalRead)(pin_size_t), int (*myAnalogRead)(pin_size_t)) :
    SlaveState()
  {
    setFromPins(myDigitalRead, myAnalogRead);
  }

#ifdef PinStatus
  // shortcut: construct directly from the pin states
  SlaveState(PinStatus (*myDigitalRead)(pin_size_t), int (*myAnalogRead)(pin_size_t)) :
    SlaveState()
  {
    setFromPins(myDigitalRead, myAnalogRead);
  }
#endif

  // assignment operator
  SlaveState& operator=(SlaveState const &s) {
    backlightDim        = s.backlightDim;
    tachometerCritical  = s.tachometerCritical;
    ignition            = s.ignition;
    fuelLevel           = s.fuelLevel;
    temperatureLevel    = s.temperatureLevel;
    oilPressureLevel    = s.oilPressureLevel;
    masterMessage       = s.masterMessage;
    CANPulseBegin       = s.CANPulseBegin;
    effectmode          = s.effectmode;
    return *this;
  }

  SlaveState(SlaveState const &s) :
    SlaveState()
  {
    SlaveState::operator=(s);
  }


} SlaveState;
