#pragma once

/**
 * This file defines the dashboard LEDs state
 */

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
