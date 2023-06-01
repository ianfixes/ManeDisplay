#include <ArduinoUnitTests.h>
#include <Wire.h>

#include "../src/SlaveProperties.h"

// mock the pin_size_t available on some boards
#ifndef pin_size_t
  typedef uint8_t pin_size_t;
#endif

int fakeDigitalRead(unsigned char pin) {
  return digitalRead(pin);
}

void fakeDigitalWrite(pin_size_t pin, int val) {
  return digitalWrite(pin, val);
}

// handle to godmode state so we can control inputs
GodmodeState* state = GODMODE();

// reset the state before every test
unittest_setup() {
  state->reset();
}

unittest(SlaveState_assignment)
{
  // assert default values
  SlaveState newstate;
  assertEqual(0,    newstate.fuelLevel);
  assertEqual(0,    newstate.temperatureLevel);
  assertEqual(0,    newstate.oilPressureLevel);
  assertEqual(0,    newstate.ignition);
  assertEqual(EffectMode::Values::none, newstate.effectmode.state);

  // assert that values can change
  SlaveState astate;
  astate.fuelLevel = 4;
  astate.temperatureLevel = 3;
  astate.oilPressureLevel = 2;
  astate.ignition = 1;
  astate.effectmode.state = EffectMode::Values::rainbow;
  assertEqual(4,    astate.fuelLevel);
  assertEqual(3,    astate.temperatureLevel);
  assertEqual(2,    astate.oilPressureLevel);
  assertEqual(1,    astate.ignition);
  assertEqual(EffectMode::Values::rainbow, astate.effectmode.state);

  // assert that assignment changes them back
  astate = newstate;
  assertEqual(0,    astate.fuelLevel);
  assertEqual(0,    astate.temperatureLevel);
  assertEqual(0,    astate.oilPressureLevel);
  assertEqual(0,    astate.ignition);
  assertEqual(EffectMode::Values::none, astate.effectmode.state);
}

unittest(SlaveState_analog_pins)
{
  // set some values on the analog pins
  state->digitalPin[SlavePin::Values::ignitionInput]   = 11; // analog pin is read with digitalRead, so mock as such
  state->analogPin[SlavePin::Values::fuelWarning]      = 22;
  state->analogPin[SlavePin::Values::fuelInput]        = 33;
  state->analogPin[SlavePin::Values::temperatureInput] = 44;
  state->analogPin[SlavePin::Values::oilInput]         = 55;
  state->analogPin[SlavePin::Values::scrollCAN]        = 66;

  // read the values
  SlaveState astate;
  astate.setFromPins(digitalRead, analogRead);

  // assert the values
  assertEqual(33,   astate.fuelLevel);
  assertEqual(44,   astate.temperatureLevel);
  assertEqual(55,   astate.oilPressureLevel);
  assertTrue(astate.ignition);
}

unittest(SlaveState_effectmode)
{
  SlaveState astate;
  assertEqual(EffectMode::Values::none, astate.effectmode.state);
}

unittest(SlaveState_scrollCAN)
{
  SlaveState astate;
  assertEqual(0, astate.CANPulseBegin);
  assertEqual(false, astate.scrollCANstate(0));
  assertEqual(false, astate.scrollCANstate(1));
  assertEqual(false, astate.scrollCANstate(49));
  assertEqual(false, astate.scrollCANstate(50));
  assertEqual(false, astate.scrollCANstate(51));

  astate.CANPulseBegin = 100;
  assertEqual(100, astate.CANPulseBegin);
  assertEqual(false, astate.scrollCANstate(99));
  assertEqual(true, astate.scrollCANstate(100));
  assertEqual(true, astate.scrollCANstate(101));
  assertEqual(true, astate.scrollCANstate(149));
  assertEqual(false, astate.scrollCANstate(150));
  assertEqual(false, astate.scrollCANstate(151));
}

unittest_main()
