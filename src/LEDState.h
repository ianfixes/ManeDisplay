#pragma once

#include "SlaveProperties.h"
#include <FastLED.h>

// Stateful LED behaviors
//
// * the LEDs keep track of their own values
// * the LED is always performing some behavior to that behavior's completion
// * when the behavior is complete, the LED picks a new behavior based on
//    the system state
// * The behavior modifies the LED's RGB or HSV profile
//
// Implementation wise, this is a finite state machine with parameterized states
// in which the stateful object maintains pre-allocated states within itself.
// Since the transitions between the states can depend on the precise responsibility
// of an individual LED, each category of LED will define its own thin extension of
// standard state base classes, just to provide the state-switching logic in a way
// that agrees with the stateful object class.  Either I'm missing an obvious
// alternate solution, or this is the only way to optimize the design, maintenance,
// and runtime performance of a system like this.

// leds[i] = CHSV((hue + (i * (255 / NUM_DASH_LEDS))) % 255, 255, 255);
// leds[i] = CRGB(255, 255, 255);

const int FLASH_DURATION_MS = 100;  // what we want for all flashing LEDs. this is 1/2 of the flash

const struct CRGB COLOR_BLACK  = CRGB::HTMLColorCode(CRGB::Black);
const struct CRGB COLOR_WHITE  = CRGB::HTMLColorCode(CRGB::White);
const struct CRGB COLOR_RED    = CRGB::HTMLColorCode(CRGB::Red);
const struct CRGB COLOR_YELLOW = CRGB::HTMLColorCode(CRGB::Yellow);
const struct CRGB COLOR_BLUE   = CRGB::HTMLColorCode(CRGB::Blue);
const struct CRGB COLOR_AMBER  = CRGB((uint32_t)0xFFBF00);


// abstract class for an LED state.
// the class's responsibility is to determine when it's OK to change state,
// and to handle the LED behavior while in the state (based on the time and
// on whatever extra member variables are provided by the child classes)
//
// note that the the LED is going to be a CRGB struct, but we can assign CHSV values to it, and
// those values will automatically be converted by FastLED's library as part of the "=" operator.
// RGB is what the hardware expects.
class LEDState {
public:
  // what to do with the LED in this state, on one tick
  virtual void loop(struct CRGB* led, unsigned long const &millis) = 0;

  // whether the state is expired.  by default, it's always time to reevaulate
  virtual bool isExpired(unsigned long const & /* millis */) const { return true; }

  // string representation
  virtual String toStringWithParams(unsigned long const & /* millis */) const = 0;

  // string representation
  virtual String toString(unsigned long const &millis) const {
    String ret = isExpired(millis) ? "E" : "_";
    ret.concat("|");
    ret.concat(toStringWithParams(millis));
    return ret;
  }

};

// a state to just show a solid color.  The color is provided on init of the state
class SolidColorState : public LEDState {
public:
  const struct CHSV m_color;

  // constructor initializes all things
  SolidColorState(const struct CHSV &hsv) : LEDState(), m_color(hsv) {}
  SolidColorState(const struct CRGB &rgb) : SolidColorState(rgb2hsv_approximate(rgb)) {}

  // the state just applies the saved color.
  // TODO: we could consider rapidly fading toward this color from whatever color was currently being displayed
  virtual void loop(struct CRGB* led, unsigned long const & /* millis */) {
    *led = m_color; // TODO: see if there's a FastLED method for doing this more natively
  }

  // The state data
  virtual String toStringWithParams(unsigned long const & /* millis */) const {
    char ret[12];
    sprintf(ret, "Sld %02X%02X%02X", m_color.h, m_color.s, m_color.v);
    return String(ret);
  }

private:
  // disallow the default constructor, so that not defining the initial color is a compiler error
  SolidColorState() {};
};

// abstract class to handle flashing; both the on and off flash states are children of this
class FlashState : public SolidColorState {
public:
  unsigned long m_startTime; // this is the timing seed for the flashing

  // same constructor as a solid color
  FlashState(const struct CHSV &hsv): SolidColorState(hsv), m_startTime(0) {}
  FlashState(const struct CRGB &rgb): FlashState(rgb2hsv_approximate(rgb)) {}

  // or just copy what's in an existing solid color state
  FlashState(SolidColorState const &s) : FlashState(s.m_color) {}

  // explicit function to modify the start time for the flash
  inline void setStartTime(unsigned long startTime) {
    m_startTime = startTime;
  }

  // state is expired when the flash period is not in the desired half
  virtual bool isExpired(unsigned long const &millis) const override {
    const int elapsedTime = millis - m_startTime;
    const int totalTime = FLASH_DURATION_MS * 2;
    // mod the time that the flash mode has been active by the flash period and determine which half we're in
    return activeOnFirstHalf() != ((elapsedTime % totalTime) < FLASH_DURATION_MS);
  }

  // whether this state should be considered active in the first half of the flash
  virtual bool activeOnFirstHalf() const = 0;

  // The state data
  virtual String toStringWithParams(unsigned long const & /* millis */) const {
    char ret[12];
    sprintf(ret, "Fl%c %02X%02X%02X", (activeOnFirstHalf() ? '1' : '2'), m_color.h, m_color.s, m_color.v);
    return String(ret);
  }
};

// the "loud" period of a flash - when the flashing light is on
class FlashLoudState : public FlashState {
public:
  FlashLoudState(const struct CHSV &hsv) : FlashState(hsv) {}
  FlashLoudState(const struct CRGB &rgb) : FlashLoudState(rgb2hsv_approximate(rgb)) {}
  FlashLoudState(SolidColorState const &s) : FlashLoudState(s.m_color) {}
  virtual bool activeOnFirstHalf() const { return true; }
};

// the quiet period of a flash - when the flashing light is off.
// explictly defining this prevents the flash from stopping and starting in a way
// that would make it appear always-on
class FlashQuietState : public FlashState {
public:
  FlashQuietState(const struct CHSV &hsv) : FlashState(hsv) {}
  FlashQuietState(const struct CRGB &rgb) : FlashQuietState(rgb2hsv_approximate(rgb)) {}
  FlashQuietState(SolidColorState const &s) : FlashQuietState(s.m_color) {}
  virtual bool activeOnFirstHalf() const { return false; }
};

// based on time and the LED index, rotate through the rainbow
class RainbowState : public LEDState {
public:
  const int m_numLEDs; // the number of LEDs in the whole strip
  const int m_index;   // the index of this LED within the strip

  RainbowState(int numLEDs, int index) : LEDState(), m_numLEDs(numLEDs), m_index(index) {}

  // get the hue as a function of time
  inline int hue(unsigned long const &millis) const {
    return ((millis / 5) + (m_index * (255 / m_numLEDs))) % 255;
  }

  virtual void loop(struct CRGB* led, unsigned long const &millis) override {
    *led = CHSV(hue(millis), 255, 255);
  }

  // The state data
  virtual String toStringWithParams(unsigned long const & millis) const {
    char ret[12];
    sprintf(ret, "Rnb    %03d", hue(millis));
    return String(ret);
  }
};


// This class defines an LED that can be in one of several states.
// The states must be defined as member variables, and a chooseNextState function
//   chooses which state to point to when it is time to find the next one.
class StatefulLED {
public:
  LEDState* m_currentState;
  struct CRGB* const m_leds; // the pointer is const, but the struct is not
  const int m_numLEDs; // the number of LEDs in the whole strip
  const int m_index;   // the index of this LED within the strip, 0-indexed

  // construct with the LEDs structure (used by fastLED), the total number of LEDs, and the index into the array
  StatefulLED(struct CRGB* leds, int numLEDs, int index) :
    m_currentState(nullptr),
    m_leds(leds),
    m_numLEDs(numLEDs),
    m_index(index)
  {}

  // shortcut to ask if we are in a given state
  inline bool inState(LEDState const &state) const {
    return &state == m_currentState;
  }

  // shortcut to ask if we are in the initial
  inline bool inInitialState() const {
    return nullptr == m_currentState;
  }

  // what state to pick next
  virtual LEDState* chooseNextState(unsigned long const &millis, const SlaveState &slave) = 0;

  // string representation of the state name
  virtual String name() const = 0;

  // string representation
  virtual String toString(unsigned long const &millis) const {
    if (inInitialState()) {
      return "[Initial]";
    }

    char ret[40];
    sprintf(ret, "[%4s %13s]", name().c_str(), m_currentState->toString(millis).c_str());
    return String(ret);
  }

  void loop(unsigned long const &millis, const SlaveState &slave) {
    if (inInitialState() || m_currentState->isExpired(millis)) {
      m_currentState = chooseNextState(millis, slave);
    }

    m_currentState->loop(m_leds + m_index, millis); // "m_leds + index" is just "&m_leds[index]"
  }
};


// A simple LED switches between a solid color mode (on/off) and a rainbow mode
// the on/off criteria cna be overridden
class SimpleLED : public StatefulLED {
public:
  RainbowState m_stRainbow;
  SolidColorState m_stOff;
  SolidColorState m_stOn;

  SimpleLED(struct CRGB* leds, int numLEDs, int index, const struct CHSV &color) :
    StatefulLED(leds, numLEDs, index),
    m_stRainbow(numLEDs, index),
    m_stOff(COLOR_BLACK),
    m_stOn(color)
  {}

  SimpleLED(struct CRGB* leds, int numLEDs, int index, const struct CRGB &color) :
    SimpleLED(leds, numLEDs, index, rgb2hsv_approximate(color))
  {}

  // the master signal for rainbow mode overrides all others
  virtual LEDState* chooseNextState(unsigned long const &millis, const SlaveState &slave) {
    if (slave.getMasterSignal(MasterSignal::Values::scrollRainbowEffects)) {
      return &m_stRainbow;
    } else {
      return isOn(millis, slave) ? &m_stOn : &m_stOff;
    }
  }

  // by default, always on
  virtual bool isOn(unsigned long const & /* millis */, const SlaveState & /* slave */) { return true; }
};

// an illumination LED is the "vanilla" tone of the whole dash
class IlluminationLED : public SimpleLED {
public:
  IlluminationLED(struct CRGB* leds, int numLEDs, int index) : SimpleLED(leds, numLEDs, index, COLOR_WHITE) {}

  // string representation of the state name
  inline virtual String name() const { return "Illu"; };
};

// control of the AC LED
class AirCondLED : public SimpleLED {
public:
  AirCondLED(struct CRGB* leds, int numLEDs, int index) : SimpleLED(leds, numLEDs, index, COLOR_BLUE) {}

  // string representation of the state name
  inline virtual String name() const { return "AC"; };

  virtual bool isOn(unsigned long const & /* millis */, const SlaveState &slave) override {
    return slave.getMasterSignal(MasterSignal::Values::acOn);
  }
};

// control of the rear window heater LED
class HeatedRearWindowLED : public SimpleLED {
public:
  HeatedRearWindowLED(struct CRGB* leds, int numLEDs, int index) : SimpleLED(leds, numLEDs, index, COLOR_YELLOW) {}

  // string representation of the state name
  inline virtual String name() const { return "Hrw"; };

  virtual bool isOn(unsigned long const & /* millis */, const SlaveState &slave) override {
    return slave.getMasterSignal(MasterSignal::Values::heatedRearWindowOn);
  }
};

// control of the rear window heater LED
class HazardLED : public SimpleLED {
public:
  HazardLED(struct CRGB* leds, int numLEDs, int index) : SimpleLED(leds, numLEDs, index, COLOR_WHITE) {}

  // string representation of the state name
  inline virtual String name() const { return "Haz"; };

  virtual bool isOn(unsigned long const & /* millis */, const SlaveState &slave) override {
    return !slave.getMasterSignal(MasterSignal::Values::hazardOff);
  }
};

// control of the rear window heater LED
class RearFoggerLED : public SimpleLED {
public:
  RearFoggerLED(struct CRGB* leds, int numLEDs, int index) : SimpleLED(leds, numLEDs, index, COLOR_AMBER) {}

  // string representation of the state name
  inline virtual String name() const { return "Fog"; };

  virtual bool isOn(unsigned long const & /* millis */, const SlaveState &slave) override {
    return slave.getMasterSignal(MasterSignal::Values::rearFoggerOn);
  }
};


// This class defines a blinking LED that can blink 2 different colors
class MultiBlinkingLED : public StatefulLED {
public:
  SolidColorState m_stSolid;
  FlashLoudState  m_stFlashRedLoud;
  FlashQuietState m_stFlashRedQuiet;
  FlashLoudState  m_stFlashAmberLoud;
  FlashQuietState m_stFlashAmberQuiet;

  // note that even though the quiet states and the solid states are the same color,
  // the solid state can be interrupted at any time

  MultiBlinkingLED(struct CRGB* leds, int numLEDs, int index) :
    StatefulLED(leds, numLEDs, index),
    m_stSolid(COLOR_WHITE),
    m_stFlashRedLoud(COLOR_RED),
    m_stFlashRedQuiet(m_stSolid),
    m_stFlashAmberLoud(COLOR_AMBER),
    m_stFlashAmberQuiet(m_stSolid)
  {}

  // conditionally seed the flash states' timing
  void seedFlashTiming(unsigned long const &millis) {
    // if we're not in one of the non-flash states, then keep the flash seed as it is.
    // this ensures that we won't restart blinking while already blinking
    if (inInitialState() || inState(m_stRainbow) || inState(m_stSolid)) {
      m_stFlashRedLoud.setStartTime(millis);
      m_stFlashRedQuiet.setStartTime(millis);
      m_stFlashAmberLoud.setStartTime(millis);
      m_stFlashAmberQuiet.setStartTime(millis);
    }
  }

  // indications for warnings and critical states
  virtual bool isWarning(const SlaveState &slave) const = 0;
  virtual bool isCritical(const SlaveState &slave) const = 0;

  // what state to pick next
  virtual LEDState* chooseNextState(unsigned long const &millis, const SlaveState &slave) {
    if (slave.getMasterSignal(MasterSignal::Values::scrollRainbowEffects)) {
      return &m_stRainbow;
    } else if (isCritical(slave)) {
      seedFlashTiming(millis);
      // loud states must transition to quiet to complete the blink. all others go loud immediately
      return (inState(m_stFlashRedLoud) || inState(m_stFlashAmberLoud)) ? (LEDState*)&m_stFlashRedQuiet : (LEDState*)&m_stFlashRedLoud;
    } else if (isWarning(slave)) {
      seedFlashTiming(millis);
      // loud states must transition to quiet to complete the blink. all others go loud immediately
      return (inState(m_stFlashRedLoud) || inState(m_stFlashAmberLoud)) ? (LEDState*)&m_stFlashAmberQuiet : (LEDState*)&m_stFlashAmberLoud;
    } else {
      return &m_stSolid;
    }
  }

};

class BoostLED : public MultiBlinkingLED {
public:
  BoostLED(struct CRGB* leds, int numLEDs, int index) : MultiBlinkingLED(leds, numLEDs, index) {}

  // string representation of the state name
  inline virtual String name() const { return "Bst"; };

  virtual bool isWarning(const SlaveState &slave) const override {
    return slave.getMasterSignal(MasterSignal::Values::boostWarning);
  }
  virtual bool isCritical(const SlaveState &slave) const override {
    return slave.getMasterSignal(MasterSignal::Values::boostCritical);
  }
};

class TachLED : public MultiBlinkingLED {
public:
  TachLED(struct CRGB* leds, int numLEDs, int index) : MultiBlinkingLED(leds, numLEDs, index) {}

  // string representation of the state name
  inline virtual String name() const { return "Tach"; };

  virtual bool isWarning(const SlaveState &slave) const override {
    return slave.tachometerWarning;
  }
  virtual bool isCritical(const SlaveState &slave) const override {
    return slave.tachometerCritical;
  }

};
