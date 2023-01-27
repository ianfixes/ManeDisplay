#pragma once

/**
 * This file defines some of the properties of the slave board
 *
 * LEDs 0-6 - tachometer backlight
 * LED 19 - Backlight LED for clock
 * LED 23 - boost
 * LED 24 - "air conditioner is powered on"
 * LED 25 - Heated Rear Window
 * LED 26 - Rear Fog Light
 * LED 27 - hazard lights
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
