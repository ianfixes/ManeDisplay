#pragma once

/**
 * This file defines some of the properties of the master board
 *
 * We create named enumerations so that the compiler can help us catch instances
 * where we might interchange a bit position with a pin number -- it will warn us
 * if we are passing a pin name to function that expects a signal name
 */

// digital pin assignments for the master
namespace MasterPin {
  enum Values {
    led23to100pctAmber   = 3,
    led23to100pctRed     = 4,
    led24to100pctBlue    = 5,
    led25to100pctYellow  = 6,
    led27toBlack         = 7,
    led26toAmber         = 8,
    d3Low                = 9,
    scrollPresetColours  = 10,
    scrollRainbowEffects = 11,
    scrollBrightness     = 12
  };
}

// different bits of information communicated by the master
namespace MasterSignal {
  enum Values {
    led23to100pctAmber   = 0,
    led23to100pctRed     = 1,
    led24to100pctBlue    = 2,
    led25to100pctYellow  = 3,
    led27toBlack         = 4,
    led26toAmber         = 5,
    d3Low                = 6,
    scrollPresetColours  = 7,
    scrollRainbowEffects = 8,
    scrollBrightness     = 9
  };
}

// min and max for the enum, for iterating
const unsigned int MASTERSIGNAL_MIN = MasterSignal::Values::led23to100pctAmber;
const unsigned int MASTERSIGNAL_MAX = MasterSignal::Values::scrollBrightness;
