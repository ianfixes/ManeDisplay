#pragma once

#include "DashMessage.h"

/**
 * This file defines the dashboard LEDs state
 *
 * LEDs 0-6 - tachometer backlight
 * LED 19 - Backlight LED for clock
 * LED 23 - boost
 * LED 24 - "air conditioner is powered on"
 * LED 25 - Heated Rear Window
 * LED 26 - Rear Fog Light
 * LED 27 - hazard lights
 */

#define COLOR_ORDER RGB
#define LED_TYPE WS2812B       // i'm using WS2811s, FastLED supports lots of different types.

#define MAX_BRIGHTNESS 255     // Thats full on, watch the power!
#define MIN_BRIGHTNESS 5       // set to a minimum of 5%

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
typedef struct DashSupport {
  void (*pinMode)(pin_size_t, int);
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


typedef struct DashState {
  DashSupport support;
  bool inBootSequence;
  DashMessage lastMessage;
  DashMessage nextMessage;
  SlaveState lastState;
  SlaveState nextState;

  struct CRGB leds[NUM_DASH_LEDS];


  // construct empty container
  DashState(DashSupport ds) {
    support = ds;
  }

  void setMessage(DashMessage dm) {
    nextMessage = dm;
  }

  void setState(SlaveState state) {
    nextState = state;
  }

  bool processBootSequence(unsigned long nMillis) {
    // TODO: actually do something with a boot sequence
    return nMillis > 3000;
  }

  void setup() {
    inBootSequence = true;
    pinMode(SlavePin::Values::scrollCAN, OUTPUT);
    pinMode(SlavePin::Values::ledStrip,  OUTPUT);

    support.fastLed->addLeds<LED_TYPE, SlavePin::Values::ledStrip, COLOR_ORDER>(leds, NUM_DASH_LEDS).setCorrection(TypicalLEDStrip);
    support.fastLed->setBrightness(MAX_BRIGHTNESS);   // TODO
  }

  // apply the internal state to the hardware
  void apply(unsigned long nMillis) {
    // perform boot sequence -- lighting check -- until sequence claims completion
    if (inBootSequence) {
      inBootSequence = processBootSequence(nMillis);
      return;
    }

    // here are the things we have to make sense of
    // TODO: delete the list when everything's crossed off it
    nextState.scrollCAN;
    nextState.backlightDim;
    nextState.tachometerCritical;
    nextState.tachometerWarning;
    nextMessage.getBit(MasterSignal::Values::boostWarning);        // TODO: Amber
    nextMessage.getBit(MasterSignal::Values::boostCritical);       // TODO: Red
    nextMessage.getBit(MasterSignal::Values::acOn);                // TODO: Blue
    nextMessage.getBit(MasterSignal::Values::heatedRearWindowOn);  // TODO: Yellow
    nextMessage.getBit(MasterSignal::Values::hazardOff);           // TODO: Black
    nextMessage.getBit(MasterSignal::Values::rearFoggerOn);        // TODO: Amber
    nextMessage.getBit(MasterSignal::Values::scrollCAN);
    nextMessage.getBit(MasterSignal::Values::scrollPresetColours);
    nextMessage.getBit(MasterSignal::Values::scrollRainbowEffects);
    nextMessage.getBit(MasterSignal::Values::scrollBrightness);


    // set the scroll CAN button state
    support.digitalWrite(SlavePin::Values::scrollCAN, nextState.scrollCAN ? HIGH : LOW);

    // TODO: set the LED state
    // leds[i] = CHSV(255, 255, 255);
    // support.fastLed->setBrightness(brightness); // not sure if there is a way (or a need) to do this per-LED
    // support.fastLed->show();

    // keep 1 step's worth of history
    lastState = nextState;
    lastMessage = nextMessage;
  }

} DashState;
