#pragma once

#include "DashMessage.h"
#include "CalibratedServo.h"
#include "LEDState.h"
#include <FastLED.h>

/**
 * This file defines the dashboard LEDs state, servos, and how the dash performs its logic
 *
 * LEDs 0-6 - tachometer backlight
 * LED 19 - Backlight LED for clock
 * LED 23 - boost
 * LED 24 - "air conditioner is powered on"
 * LED 25 - Heated Rear Window
 * LED 26 - Rear Fog Light
 * LED 27 - hazard lights
 */

#define COLOR_ORDER GRB
#define LED_TYPE WS2812B       // i'm using WS2811s, FastLED supports lots of different types.

// define limits for sensor inputs
const Range fuelSenderLimit { 0, 1023 };
const Range tempSenderLimit { 0, 1023 };
const Range oilSenderLimit  { 0, 1023 };

// define limits for servo outputs
const Range fuelServoLimit  { 0, 180 };
const Range tempServoLimit  { 0, 180 };
const Range oilServoLimit   { 0, 180 };

// define limits for LED strip brightness
const Range LEDStripBrightnessLimit { 5, 255 };
const int dimBrightnessLevel = (LEDStripBrightnessLimit.max - LEDStripBrightnessLimit.min) / 2;

const unsigned int ARDUINO_BOOT_ANIMATION_MS = 2000; // amount of time that we can use to do a bootup sequence
const unsigned int ARDUINO_SOFT_SHUTDOWN_MS = 3000; // amount of time that we can use to do a soft shutdown

// LED assignments across the dash. numbers favor left to right reading
namespace DashLED {
  enum Values {
    tach0               = 0,
    tach1               = 1,
    tach2               = 2,
    tach3               = 3,
    tach4               = 4,
    tach5               = 5,
    tach6               = 6,
    gauge1              = 7,
    gauge0              = 8,
    CAN                 = 9,
    gauge3              = 10,
    gauge2              = 11,
    speed0              = 12,
    speed1              = 13,
    speed2              = 14,
    speed3              = 15,
    speed4              = 16,
    speed5              = 17,
    speed6              = 18,
    clock               = 19,
    oilDial             = 20,
    boostDial           = 21,
    voltsDial           = 22,
    boostInd            = 23,
    airConditioningInd  = 24,
    heatedRearWindowInd = 25,
    rearFogLightInd     = 26,
    hazardInd           = 27,
    auxLight            = 28,
    heater0             = 29,
    heater1             = 30,
    windowSw1           = 31,
    windowSw0           = 32
  };
}

// min and max for the enum, for iterating
const unsigned int DASH_LED_MIN = DashLED::Values::tach0;
const unsigned int DASH_LED_MAX = DashLED::Values::windowSw0;
const unsigned int NUM_DASH_LEDS = DASH_LED_MAX + 1;





// Struct to dependency-inject any standard functions needed
// This is a way to separate the function of these classes from the hardware,
// which will make testing easier -- we can feed it mock functions if we want
typedef struct DashSupport {
  void (*pinMode)(pin_size_t, int);
  int (*analogRead)(unsigned char);
  int (*digitalRead)(unsigned char);
  void (*digitalWrite)(pin_size_t, int);
  CFastLED* fastLed;
} DashSupport;






// define the LED position in physical space,
// relative to upper left of console,
// in millimeters of offset rightward and downward
// ... eventually
typedef struct LEDPosition {
  unsigned int x;
  unsigned int y;
} LEDPosition;

// taking x/y from layout.jpg in pixels.
// obviously those positions aren't real-world,
// but they will help us find out whether any
// effects based on position are compelling
const LEDPosition ledPosition[NUM_DASH_LEDS] = {
  {2023,  551},  // tach0               00
  {2071,  435},  // tach1               01
  {2023,  321},  // tach2               02
  {1907,  274},  // tach3               03
  {1792,  321},  // tach4               04
  {1743,  435},  // tach5               05
  {1792,  551},  // tach6               06
  {1633,  323},  // gauge1              07
  {1676,  407},  // gauge0              08
  {1524,  424},  // CAN                 09
  {1370,  407},  // gauge3              10
  {1412,  323},  // gauge2              11
  {1255,  551},  // speed0              12
  {1303,  435},  // speed1              13
  {1255,  321},  // speed2              14
  {1140,  274},  // speed3              15
  {1024,  321},  // speed4              16
  { 976,  435},  // speed5              17
  {1024,  551},  // speed6              18
  { 573,   90},  // clock               19
  { 702,  255},  // oilDial             20
  { 437,  255},  // boostDial           21
  { 174,  255},  // voltsDial           22
  { 252,  937},  // boostInd            23
  { 384,  937},  // airConditioningInd  24
  { 516,  937},  // heatedRearWindowInd 25
  { 650,  937},  // rearFogLightInd     26
  { 781,  937},  // hazardInd           27
  { 913,  937},  // auxLight            28
  { 463, 1290},  // heater0             29
  { 696, 1290},  // heater1             30
  { 639, 1428},  // windowSw1           31
  { 529, 1428}   // windowSw0           32
};




// The state of the dashboard.
//
// This struct is responsible for configuring the hardware initially
// and, given input in the form of:
//   * a hardware state
//   * the latest message from I2C
//   * the current time, in millis
// make all decisions about what the output should look like, and
// apply that output.
typedef struct DashState {
  DashSupport support;
  SlaveState lastState;
  SlaveState nextState;

  struct CRGB leds[NUM_DASH_LEDS];

  CalibratedServo fuelGauge;
  CalibratedServo tempGauge;
  CalibratedServo oilGauge;

  unsigned long bootStartTime;
  unsigned long ignitionLastOnTime;

  // can't declare an array of abstract classes, so declare an array
  // of pointers to those abstract classes.  hence the use of "new".
  StatefulLED* statefulLeds[NUM_DASH_LEDS] = {
    new             TachLED(leds, NUM_DASH_LEDS, DashLED::Values::tach0),
    new             TachLED(leds, NUM_DASH_LEDS, DashLED::Values::tach1),
    new             TachLED(leds, NUM_DASH_LEDS, DashLED::Values::tach2),
    new             TachLED(leds, NUM_DASH_LEDS, DashLED::Values::tach3),
    new             TachLED(leds, NUM_DASH_LEDS, DashLED::Values::tach4),
    new             TachLED(leds, NUM_DASH_LEDS, DashLED::Values::tach5),
    new             TachLED(leds, NUM_DASH_LEDS, DashLED::Values::tach6),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::gauge1),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::gauge0),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::CAN),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::gauge3),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::gauge2),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::speed0),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::speed1),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::speed2),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::speed3),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::speed4),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::speed5),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::speed6),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::clock),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::oilDial),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::boostDial),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::voltsDial),
    new            BoostLED(leds, NUM_DASH_LEDS, DashLED::Values::boostInd),
    new          AirCondLED(leds, NUM_DASH_LEDS, DashLED::Values::airConditioningInd),
    new HeatedRearWindowLED(leds, NUM_DASH_LEDS, DashLED::Values::heatedRearWindowInd),
    new       RearFoggerLED(leds, NUM_DASH_LEDS, DashLED::Values::rearFogLightInd),
    new           HazardLED(leds, NUM_DASH_LEDS, DashLED::Values::hazardInd),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::auxLight),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::heater0),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::heater1),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::windowSw1),
    new     IlluminationLED(leds, NUM_DASH_LEDS, DashLED::Values::windowSw0)
  };

  // construct empty container
  // This is also where we set the calibration data for the servos
  DashState(DashSupport ds):
    support(ds),
    fuelGauge(SlavePin::Values::fuelServo, fuelSenderLimit, fuelServoLimit),
    tempGauge(SlavePin::Values::tempServo, tempSenderLimit, tempServoLimit),
    oilGauge( SlavePin::Values::oilServo,  oilSenderLimit,  oilServoLimit),
    bootStartTime(0),
    ignitionLastOnTime(0)
  {}

  // accept a message from I2C
  void setMessage(DashMessage const &dm) {
    nextState.setMasterSignals(dm);
  }

  // accept a hardware state
  void setState(SlaveState state) {
    state.masterMessage = nextState.masterMessage; // carry over current message state
    nextState = state;
  }

  // measure the time since the first measured time
  inline bool inBootSequence(unsigned long const &nMillis) const {
    return (nMillis - bootStartTime) < ARDUINO_BOOT_ANIMATION_MS;
  }

  // scripted startup animation
  void processBootSequence(unsigned long const &nMillis) {
    // turn up the gauges all the way to show that they work
    fuelGauge.writeMax();
    tempGauge.writeMax();
    oilGauge.writeMax();

    // linearly ramp up the backlight brightness over the boot time
    const int initialBrightness = lastState.backlightDim ? dimBrightnessLevel : LEDStripBrightnessLimit.max;
    const int rampedBrightness = map(nMillis - bootStartTime,
      0, ARDUINO_BOOT_ANIMATION_MS,
      LEDStripBrightnessLimit.min, initialBrightness
    );
    support.fastLed->setBrightness(rampedBrightness);
    support.fastLed->show();
  }

  // scripted shutdown animation
  void processShutdownSequence(unsigned long const &nMillis) {
    // linearly ramp down the backlight brightness over the soft shutdown time
    const int initialBrightness = lastState.backlightDim ? dimBrightnessLevel : LEDStripBrightnessLimit.max;
    const int rampedBrightness = map(ARDUINO_SOFT_SHUTDOWN_MS - (nMillis - ignitionLastOnTime),
      0, ARDUINO_SOFT_SHUTDOWN_MS,
      LEDStripBrightnessLimit.min, initialBrightness
    );
    support.fastLed->setBrightness(rampedBrightness);
    support.fastLed->show();

    // park all servos
    fuelGauge.writeMin();
    tempGauge.writeMin();
    oilGauge.writeMin();
  }

  // decide whether the optocoupler should be employed based on time and ignition state
  inline bool shouldUseOpto(bool ignitionIsOn, unsigned long const &nMillis) const {
    if (ignitionIsOn) return false; // explictly make sure that we never never cross the streams
    return (nMillis - ignitionLastOnTime) < ARDUINO_SOFT_SHUTDOWN_MS;
  }

  // perform all hardware setup and software state init for this board
  void setup() {
    // configure inputs
    SlaveState::setup(support.pinMode);

    // configure outputs
    support.pinMode(SlavePin::Values::scrollCAN, OUTPUT);
    support.pinMode(SlavePin::Values::ledStrip,  OUTPUT);
    fuelGauge.setup();
    tempGauge.setup();
    oilGauge.setup();

    support.fastLed->addLeds<LED_TYPE, SlavePin::Values::ledStrip, COLOR_ORDER>(leds, NUM_DASH_LEDS).setCorrection(TypicalLEDStrip);
  }

  // convert the dash state to something we can monitor
  String toString(unsigned long const &nMillis, SlaveState const &slaveState) const {
    String ret = "[";
    ret.concat(!slaveState.ignition ? "HALT" : (inBootSequence(nMillis) ? "BOOT" : " OK "));
    ret.concat("] ");
    ret.concat(slaveState.toString());

    // include all stateful LEDs
    for (unsigned int i = DASH_LED_MIN; i < NUM_DASH_LEDS; ++i) {
      ret.concat(i % 7 == 0 ? "\n" : " ");
      ret.concat(statefulLeds[i]->toString(nMillis));
    }

    return ret;
  }

  // convert the last dash state to string
  inline String lastStateString(unsigned long const &nMillis) const {
    return toString(nMillis, lastState);
  }

  //convert the next dash state to string
  inline String nextStateString(unsigned long const &nMillis) const {
    return toString(nMillis, nextState);
  }

  // apply the internal state to the hardware
  void apply(unsigned long const &nMillis) {
    // DATA SAFETY SECTION: ensure state data isn't corrupted
    lastState = nextState; // try to keep the async 2wire receiver from interfering with current state
    if (0 == bootStartTime) bootStartTime = nMillis; // get a real measure of boot start time


    // TODO: delete the list when everything's crossed off it
    // here are the things we have to make sense of
    lastState.getMasterSignal(MasterSignal::Values::scrollPresetColours);
    lastState.getMasterSignal(MasterSignal::Values::scrollBrightness);


    // EXISTENTIAL SECTION: ensure board is powered when we want power
    digitalWrite(SlavePin::Values::optoCoupler, shouldUseOpto(lastState.ignition, nMillis));

    // GRACEFUL EXIT SECTION: perform shutdown animation/tasks if we're in shutdown, and nothing more
    if (!lastState.ignition) {
      processShutdownSequence(nMillis);
      return;
    }

    // STUFF ALLOWED DURING BOOT SECTION:
    // update the scroll CAN button state
    support.digitalWrite(SlavePin::Values::scrollCAN, lastState.scrollCAN ? HIGH : LOW);

    // update all stateful LEDs from the input. this will mean they're always the right hue
    for (unsigned int i = DASH_LED_MIN; i < NUM_DASH_LEDS; ++i) {
      statefulLeds[i]->loop(millis, lastState);
    }

    // BOOT SEQUENCE SECTION: perform boot animation if we're in boot, and nothing more
    if (inBootSequence(nMillis)) {
      processBootSequence(nMillis);
      return;
    }

    // STEADY STATE SECTION: from here on out, behave normally.

    // update the servos
    fuelGauge.write(lastState.fuelLevel);
    tempGauge.write(lastState.temperatureLevel);
    oilGauge.write(lastState.oilPressureLevel);

    // update the overall LED strip brightness according to dimmer signal
    support.fastLed->setBrightness(lastState.backlightDim ? dimBrightnessLevel : LEDStripBrightnessLimit.max);
    support.fastLed->show();
  }

} DashState;
