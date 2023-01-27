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
    boostWarning         = 3,
    boostCritical        = 4,
    acOn                 = 5,
    heatedRearWindowOn   = 6,
    hazardOff            = 7,
    rearFoggerOn         = 8,
    scrollCAN            = 9,
    scrollPresetColours  = 10,
    scrollRainbowEffects = 11,
    scrollBrightness     = 12
  };
}

// different bits of information communicated by the master
namespace MasterSignal {
  enum Values {
    boostWarning         = 0,
    boostCritical        = 1,
    acOn                 = 2,
    heatedRearWindowOn   = 3,
    hazardOff            = 4,
    rearFoggerOn         = 5,
    scrollCAN            = 6,
    scrollPresetColours  = 7,
    scrollRainbowEffects = 8,
    scrollBrightness     = 9
  };
}

// min and max for the enum, for iterating
const unsigned int MASTERSIGNAL_MIN = MasterSignal::Values::boostWarning;
const unsigned int MASTERSIGNAL_MAX = MasterSignal::Values::scrollBrightness;
