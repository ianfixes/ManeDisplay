#include <ArduinoUnitTests.h>
#include <Wire.h>

#include "../src/DashMessage.h"
#include "../src/DashState.h"


// mock a FastLED object
CFastLED FastLED;

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

// dummy dash support, dependency injection
DashSupport ds = {
  pinMode,
  analogRead,
  fakeDigitalRead,
  fakeDigitalWrite,
  &FastLED
};

// handle to godmode state so we can control inputs
GodmodeState* state = GODMODE();
DashState dash(ds);

// reset the state before every test
unittest_setup() {
  state->reset();
  dash.setup();
}

unittest(DashState_init)
{
  // show that dash setup() does in fact initialize everything
  state->digitalPin[SlavePin::Values::ignitionInput]   = 0; // analog pin is read with digitalRead, so mock as such
  assertEqual(0,    dash.bootStartTime);
  assertEqual(0,    dash.ignitionLastOnTime);

  // apply boot time should change the state
  dash.apply(11);
  assertEqual(11,   dash.bootStartTime);
  assertEqual(0,    dash.ignitionLastOnTime);

  // rerunning setup should change it back
  dash.reset();
  assertEqual(0,    dash.bootStartTime);
  assertEqual(0,    dash.ignitionLastOnTime);
}

unittest(boot_time)
{
  // initial state is zeros
  assertEqual(0,    dash.bootStartTime);
  assertEqual(0,    dash.ignitionLastOnTime);

  // check that the boot time comes on but not the ignition
  state->digitalPin[SlavePin::Values::ignitionInput] = 0;
  dash.setSlaveState(digitalRead, analogRead);
  dash.apply(11);
  assertEqual(11,   dash.bootStartTime);
  assertEqual(0,    dash.ignitionLastOnTime);

  // check that the ignition takes effect at the appropriate time
  state->digitalPin[SlavePin::Values::ignitionInput] = 55;
  dash.setSlaveState(digitalRead, analogRead);
  dash.apply(22);
  assertEqual(11,   dash.bootStartTime);
  assertEqual(22,   dash.ignitionLastOnTime);

}

unittest(digital_raw_signals_to_state)
{
  for (int i = 0; i < 2; ++i) {
    state->reset();
    dash.setup();

    DashMessage dm;
    dm.setBit(MasterSignal::Values::boostWarning,         i);
    dm.setBit(MasterSignal::Values::boostCritical,        i);
    dm.setBit(MasterSignal::Values::acOn,                 i);
    dm.setBit(MasterSignal::Values::heatedRearWindowOn,   i);
    dm.setBit(MasterSignal::Values::hazardOff,            i);
    dm.setBit(MasterSignal::Values::rearFoggerOn,         i);
    dm.setBit(MasterSignal::Values::scrollCAN,            i);
    dm.setBit(MasterSignal::Values::scrollPresetColours,  i);
    dm.setBit(MasterSignal::Values::scrollRainbowEffects, i);
    dm.setBit(MasterSignal::Values::scrollBrightness,     i);
    dash.setMessage(dm);

    state->digitalPin[SlavePin::Values::ignitionInput]      = 11;
    state->digitalPin[SlavePin::Values::backlightDim]       = i;
    state->digitalPin[SlavePin::Values::tachometerCritical] = i;
    state->digitalPin[SlavePin::Values::tachometerWarning]  = i;

    dash.setSlaveState(digitalRead, analogRead);

    //assertEqual(i,    dash.state().scrollCAN);
    assertEqual(i,    dash.state().backlightDim);
    assertEqual(i,    dash.state().tachometerCritical);
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::boostWarning));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::boostCritical));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::acOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::heatedRearWindowOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::hazardOff));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::rearFoggerOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollCAN));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollPresetColours));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollRainbowEffects));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollBrightness));
    assertEqual(i,    dash.state().effectmode.state);

    dash.apply(35);

    //assertEqual(i,    dash.state().scrollCAN);
    assertEqual(i,    dash.state().backlightDim);
    assertEqual(i,    dash.state().tachometerCritical);
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::boostWarning));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::boostCritical));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::acOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::heatedRearWindowOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::hazardOff));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::rearFoggerOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollCAN));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollPresetColours));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollRainbowEffects));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollBrightness));
    assertEqual(i,    dash.state().effectmode.state);
  }
}

unittest(digital_debounced_signals_to_state)
{
  for (int i = 0; i < 2; ++i) {
    state->reset();
    dash.setup();

    DashMessage dm;
    dm.setBit(MasterSignal::Values::boostWarning,         i);
    dm.setBit(MasterSignal::Values::boostCritical,        i);
    dm.setBit(MasterSignal::Values::acOn,                 i);
    dm.setBit(MasterSignal::Values::heatedRearWindowOn,   i);
    dm.setBit(MasterSignal::Values::hazardOff,            i);
    dm.setBit(MasterSignal::Values::rearFoggerOn,         i);
    dm.setBit(MasterSignal::Values::scrollCAN,            i);
    dm.setBit(MasterSignal::Values::scrollPresetColours,  i);
    dm.setBit(MasterSignal::Values::scrollRainbowEffects, i);
    dm.setBit(MasterSignal::Values::scrollBrightness,     i);
    dash.setMessage(dm);

    state->digitalPin[SlavePin::Values::ignitionInput]      = 11;
    state->digitalPin[SlavePin::Values::backlightDim]       = i;
    state->digitalPin[SlavePin::Values::tachometerCritical] = i;
    state->digitalPin[SlavePin::Values::tachometerWarning]  = i;

    dash.setSlaveState(digitalRead, analogRead);

    assertEqual(i,    dash.state().scrollCAN);
    assertEqual(i,    dash.state().backlightDim);
    assertEqual(i,    dash.state().tachometerCritical);
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::boostWarning));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::boostCritical));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::acOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::heatedRearWindowOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::hazardOff));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::rearFoggerOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollCAN));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollPresetColours));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollRainbowEffects));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollBrightness));
    assertEqual(i,    dash.state().effectmode.state);

    dash.apply(35);

    assertEqual(i,    dash.state().scrollCAN);
    assertEqual(i,    dash.state().backlightDim);
    assertEqual(i,    dash.state().tachometerCritical);
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::boostWarning));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::boostCritical));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::acOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::heatedRearWindowOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::hazardOff));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::rearFoggerOn));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollCAN));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollPresetColours));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollRainbowEffects));
    assertEqual(i,    dash.state().masterMessage.getBit(MasterSignal::Values::scrollBrightness));
    assertEqual(i,    dash.state().effectmode.state);
  }
}




  // state->analogPin[1] = 99;
  // assertEqual(99, analogRead(1));
  // state->analogPin[1] = 56;
  // assertEqual(56, analogRead(1));

  // int future[6] = {33, 22, 55, 11, 44, 66};
  // state->analogPin[1].fromArray(future, 6);
  // for (int i = 0; i < 6; ++i)
  // {
  //   assertEqual(future[i], analogRead(1));
  // }

  // // assert end of history works
  // assertEqual(future[5], analogRead(1));


unittest_main()
