#pragma once

#include "SlaveProperties.h"

#ifndef ARDUINO_CI_COMPILATION_MOCKS
  #include <FastLED.h>
#else
  #include <FakeFastLED.h>
#endif

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


// define the LED position in physical space,
// relative to upper left of console,
// in millimeters of offset rightward and downward
// ... eventually
struct LEDPosition {
  unsigned int x;
  unsigned int y;
};

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
  unsigned long m_activationTimeMs;

  // perform activation
  void activate(unsigned long const &millis) {
    m_activationTimeMs = millis;
    activateLocal();
  }

  // perform any specific activation
  virtual void activateLocal() { }

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
  virtual void loop(struct CRGB* led, unsigned long const & /* millis */) override {
    *led = m_color; // TODO: see if there's a FastLED method for doing this more natively
  }

  // The state data
  virtual String toStringWithParams(unsigned long const & /* millis */) const override {
    char ret[12];
    sprintf(ret, "Sld %02X%02X%02X", m_color.h, m_color.s, m_color.v);
    return String(ret);
  }

private:
  // disallow the default constructor, so that not defining the initial color is a compiler error
  SolidColorState() {};
};

// a state to just show a solid color.  The color is provided on init of the state
class SolidColorTimedState : public SolidColorState {
public:
  unsigned long const m_lifetimeMs;
  unsigned long m_expiryTimeMs;

  SolidColorTimedState(const struct CHSV &hsv, int lifetimeMs) : SolidColorState(hsv), m_lifetimeMs(lifetimeMs), m_expiryTimeMs(0) {}
  SolidColorTimedState(const struct CRGB &rgb, int lifetimeMs) : SolidColorTimedState(rgb2hsv_approximate(rgb), lifetimeMs) {}

  // The state data
  virtual String toStringWithParams(unsigned long const &millis) const override {
    char ret[12];
    sprintf(ret, "Slt %02X %03d", m_color.h, (int)((m_expiryTimeMs - millis) % 1000));
    return String(ret);
  }

  // set the expiry clock when the state activates
  virtual void activateLocal() override {
    m_expiryTimeMs = m_activationTimeMs + m_lifetimeMs;
  }

  // observe the expiry clock
  virtual bool isExpired(unsigned long const &millis) const override {
    return m_expiryTimeMs < millis;
  }
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
  virtual String toStringWithParams(unsigned long const & /* millis */) const override {
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
  virtual String toStringWithParams(unsigned long const & millis) const override {
    char ret[12];
    sprintf(ret, "Rnb    %03d", hue(millis));
    return String(ret);
  }
};

// Flash briefly and then wait a random amount of time.  Taken together, it's sparkly
class SparkleState : public LEDState {
public:
  unsigned long m_canFlashMs;
  const unsigned int m_sparkleDurationMs = 20;

  SparkleState() : LEDState(), m_canFlashMs(0) {}

  // on-time is in the future
  inline bool beforeFlash(unsigned long const &millis) const {
    return millis < m_canFlashMs;
  }

  // on-time and off-time have both elapsed
  inline bool afterFlash(unsigned long const &millis) const {
    return (m_canFlashMs + m_sparkleDurationMs) < millis;
  }

  // before the pulse, go dark. during the pulse, go light. after the pulse, pick a new pulse time.
  virtual void loop(struct CRGB* led, unsigned long const &millis) override {
    if (beforeFlash(millis)) {
      *led = COLOR_BLACK;
    } else if (afterFlash(millis)) {
      m_canFlashMs = millis + m_sparkleDurationMs + random(FLASH_DURATION_MS * 5);
    } else {
      *led = COLOR_WHITE;
    }
  }

  // The state data
  virtual String toStringWithParams(unsigned long const & millis) const override {
    char ret[12];
    if (beforeFlash(millis)) {
      sprintf(ret, "Sprk  %04ld", m_canFlashMs - millis);
    } else if (afterFlash(millis)) {
      sprintf(ret, "SPRK  ----");
    } else {
      sprintf(ret, "SPRK  %04ld", m_canFlashMs + m_sparkleDurationMs - millis);
    }

    return String(ret);
  }
};

// Simulate a position-based shimmer of the LEDs
//
// We will sweep an imaginary line across the LEDs in-situ, and based on their
// distance to that imaginary line, we will light them proprotionally.
class ShimmerState : public LEDState {
public:
  const struct LEDPosition m_pos;
  const unsigned int m_shimmerDurationMs;  // this is how long we want the effect to take
  const float m_distanceFactor;            // the bigger this is, the narrower the shimmer
  const unsigned int m_shimmerSpeedFactor; // the bigger this is, the faster the shimmer goes across
  const long m_initialHeight;              // the higher this is (negative scale), the longer the line takes to reach the LEDs
  const float m_slope;                     // the slope of the imaginary line

  // full featured constructor, for unit testing
  ShimmerState(
    const struct LEDPosition pos,
    unsigned int shimmerDurationMs,
    float distanceFactor,
    unsigned int shimmerSpeedFactor,
    long initialHeight,
    double slope
  ) :
    LEDState(),
    m_pos(pos),
    m_shimmerDurationMs(shimmerDurationMs),
    m_distanceFactor(distanceFactor),
    m_shimmerSpeedFactor(shimmerSpeedFactor),
    m_initialHeight(initialHeight),
    m_slope(slope)
  {}

  // minimal constructor with actual defaults in use
  ShimmerState(const struct LEDPosition pos) : ShimmerState(pos, 6500, 0.025, 8, -6000, 1) {}
  // Slow motion for testing
  // ShimmerState(const struct LEDPosition pos) : ShimmerState(pos, 65000, 0.025, 1, -4000, 1) {}

  // this defines how sharply the brightness falls off with distance.
  // we're using an inverse square law here.
  inline unsigned int velVsDistance(unsigned long const &distance) const {
    return max(0, 255 - pow(distance * m_distanceFactor, 2));
  }

  // control the sweep of the imaginary line
  inline long animationPosition(unsigned long const &millis) const {
    return (((millis - m_activationTimeMs) % m_shimmerDurationMs) * m_shimmerSpeedFactor);
  }

  // imaginary line moves with respect to time, from low X to high X
  inline unsigned long linearDistance(unsigned long const &millis) const {
    return abs(2000 + m_pos.x - animationPosition(millis));
  }

  // Imaginary line with a slope moves from high Y to low Y, but considers M and X
  inline unsigned long distanceToLine(unsigned long const &millis) const {
    const double m(m_slope);
    long b = m_initialHeight + animationPosition(millis);

    // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
    return abs((m * m_pos.x) - m_pos.y + b) / sqrt((m * m_slope) + 1);
  }

  // get the vel as a function of time, which in turn is a function of distnace to imaginary line
  inline unsigned int vel(unsigned long const &millis) const {
    //return velVsDistance(linearDistance(millis));
    return velVsDistance(distanceToLine(millis));
  }

  // before the pulse, go dark. during the pulse, go light. after the pulse, pick a new pulse time.
  virtual void loop(struct CRGB* led, unsigned long const &millis) override {
    *led = CHSV(0, 0, vel(millis));
  }

  // The state data
  virtual String toStringWithParams(unsigned long const & millis) const override {
    char ret[12];
    sprintf(ret, "Shim  %04ld", millis % m_shimmerDurationMs);
    return String(ret);
  }
};

////////////////////////////////////////////////////////////////////
//
// LED Behavior Types
//
// Individual states can be combined in some broad categories to define behaviors.
// E.g. some lights blink sometimes, others directly indicate an input signal.
//

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
  StatefulLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) :
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

  // on each iteation, check if the state has expired and activate the next one if so
  void loop(unsigned long const &millis, const SlaveState &slave) {
    if (inInitialState() || m_currentState->isExpired(millis)) {
      LEDState* newState = chooseNextState(millis, slave);
      if (newState != m_currentState) {
        m_currentState = newState;
        m_currentState->activate(millis);
      }
    }

    m_currentState->loop(m_leds + m_index, millis); // "m_leds + index" is just "&m_leds[index]"
  }
};


// All Binky LEDs respond to global modes (like rainbow)
// The local states must be defined as member variables, and a chooseNextLocalState function
//   chooses which state to point to when it is time to find the next one.  This ensures
//   we only need to handle global behaviors here.
class BinkyLED : public StatefulLED {
public:
  SolidColorTimedState m_stOn;
  SolidColorState m_stOff;
  RainbowState m_stRainbow;
  SparkleState m_stSparkle;
  ShimmerState m_stShimmer;

  BinkyLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) :
    StatefulLED(leds, ledPosition, numLEDs, index),
    m_stOn(COLOR_WHITE, FLASH_DURATION_MS / 10),
    m_stOff(COLOR_BLACK),
    m_stRainbow(numLEDs, index),
    m_stSparkle(),
    m_stShimmer(ledPosition[index])
  {}

  // the master signal for rainbow modes override all others because they work as a group
  virtual LEDState* chooseNextState(unsigned long const &millis, const SlaveState &slave) {
    switch (slave.effectmode.state) {
    case EffectMode::Values::rainbow:
      return &m_stRainbow;
    case EffectMode::Values::sparkle:
      return &m_stSparkle;
    case EffectMode::Values::shimmer:
      return &m_stShimmer;
    default:
      return chooseNextLocalState(millis, slave);
    }
  }

  // what state to pick next
  virtual LEDState* chooseNextLocalState(unsigned long const &millis, const SlaveState &slave) = 0;
};

// A simple LED switches between a solid color mode (on/off) and a rainbow mode
// the on/off criteria can be overridden
class SimpleLED : public BinkyLED {
public:
  SolidColorState m_stOff;
  SolidColorState m_stOn;

  SimpleLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index, const struct CHSV &color) :
    BinkyLED(leds, ledPosition, numLEDs, index),
    m_stOff(COLOR_BLACK),
    m_stOn(color)
  {}

  SimpleLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index, const struct CRGB &color) :
    SimpleLED(leds, ledPosition, numLEDs, index, rgb2hsv_approximate(color))
  {}

  // the master signal for rainbow mode overrides all others
  virtual LEDState* chooseNextLocalState(unsigned long const &millis, const SlaveState &slave) {
    return isOn(millis, slave) ? &m_stOn : &m_stOff;
  }

  // by default, always on
  virtual bool isOn(unsigned long const & /* millis */, const SlaveState & /* slave */) { return true; }
};

// an illumination LED is the "vanilla" tone of the whole dash
class IlluminationLED : public SimpleLED {
public:
  IlluminationLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) : SimpleLED(leds, ledPosition, numLEDs, index, COLOR_WHITE) {}

  // string representation of the state name
  inline virtual String name() const override { return "Illu"; };
};

// control of the AC LED
class AirCondLED : public SimpleLED {
public:
  AirCondLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) : SimpleLED(leds, ledPosition, numLEDs, index, COLOR_BLUE) {}

  // string representation of the state name
  inline virtual String name() const override { return "AC"; };

  virtual bool isOn(unsigned long const & /* millis */, const SlaveState &slave) override {
    return slave.getMasterSignal(MasterSignal::Values::acOn);
  }
};

// control of the rear window heater LED
class HeatedRearWindowLED : public SimpleLED {
public:
  HeatedRearWindowLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) : SimpleLED(leds, ledPosition, numLEDs, index, COLOR_YELLOW) {}

  // string representation of the state name
  inline virtual String name() const override { return "Hrw"; };

  virtual bool isOn(unsigned long const & /* millis */, const SlaveState &slave) override {
    return slave.getMasterSignal(MasterSignal::Values::heatedRearWindowOn);
  }
};

// control of the rear window heater LED
class HazardLED : public SimpleLED {
public:
  HazardLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) : SimpleLED(leds, ledPosition, numLEDs, index, COLOR_WHITE) {}

  // string representation of the state name
  inline virtual String name() const override { return "Haz"; };

  virtual bool isOn(unsigned long const & /* millis */, const SlaveState &slave) override {
    return !slave.getMasterSignal(MasterSignal::Values::hazardOff);
  }
};

// control of the rear window heater LED
class RearFoggerLED : public SimpleLED {
public:
  RearFoggerLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) : SimpleLED(leds, ledPosition, numLEDs, index, COLOR_AMBER) {}

  // string representation of the state name
  inline virtual String name() const override { return "Fog"; };

  virtual bool isOn(unsigned long const & /* millis */, const SlaveState &slave) override {
    return slave.getMasterSignal(MasterSignal::Values::rearFoggerOn);
  }
};


// This class defines a blinking LED that can blink 2 different colors
class MultiBlinkingLED : public BinkyLED {
public:
  SolidColorState m_stSolid;
  FlashLoudState  m_stFlashRedLoud;
  FlashQuietState m_stFlashRedQuiet;
  FlashLoudState  m_stFlashAmberLoud;
  FlashQuietState m_stFlashAmberQuiet;

  // note that even though the quiet states and the solid states are the same color,
  // the solid state can be interrupted at any time

  MultiBlinkingLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) :
    BinkyLED(leds, ledPosition, numLEDs, index),
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
  virtual LEDState* chooseNextLocalState(unsigned long const &millis, const SlaveState &slave) {
    if (isCritical(slave)) {
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
  BoostLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) : MultiBlinkingLED(leds, ledPosition, numLEDs, index) {}

  // string representation of the state name
  inline virtual String name() const override { return "Bst"; };

  virtual bool isWarning(const SlaveState &slave) const override {
    return slave.getMasterSignal(MasterSignal::Values::boostWarning);
  }
  virtual bool isCritical(const SlaveState &slave) const override {
    return slave.getMasterSignal(MasterSignal::Values::boostCritical);
  }
};

class TachLED : public MultiBlinkingLED {
public:
  TachLED(struct CRGB* leds, const struct LEDPosition* ledPosition, int numLEDs, int index) : MultiBlinkingLED(leds, ledPosition, numLEDs, index) {}

  // string representation of the state name
  inline virtual String name() const override { return "Tach"; };

  virtual bool isWarning(const SlaveState &slave) const override {
    return slave.tachometerWarning;
  }
  virtual bool isCritical(const SlaveState &slave) const override {
    return slave.tachometerCritical;
  }

};
