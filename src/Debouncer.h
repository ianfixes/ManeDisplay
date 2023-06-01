#pragma once

// This construct is about reducing the frequency of events -- we ignore some state changes
// until we are sure they are stable.  This function will emit one event per stable state change
// in the form of a trinary
typedef struct Debouncer {

  // we only care about the state changes
  enum Event {
    none   = 0,
    toLow  = 1,
    toHigh = 2,
  };

  unsigned int const stableTime;
  unsigned long lastStableTime;  // the last time the state changed
  bool stableState;
  bool lastReading;
  Event last;

  Debouncer(unsigned int debounceDelay) :
    stableTime(debounceDelay),
    lastStableTime(0),
    stableState(false),
    lastReading(false),
    last(Event::none)
  {}

  // force the debouncer into an initial state without generating an event
  void force(bool desiredState) {
    lastStableTime = 0;
    stableState = desiredState;
  }

  // accept a new input value for debouncing
  void process(unsigned long const &millis, bool reading) {
    last = Event::none;                                    // default: nothing to see here
    if (reading != lastReading) lastStableTime = millis;   // set "time since last changed"
    if ((millis - lastStableTime) >= stableTime) {         // guard against recent changes
      if (stableState != reading) {                        // detect changes
        stableState = reading;                             // accept changes
        last = stableState ? Event::toHigh : Event::toLow; // select event of change
      }
    }
    lastReading = reading;                                 // prepare for next loop
  }

  // accept a new input value and return any event it generated
  Event eventOf(unsigned long const &millis, bool reading) {
    process(millis, reading);
    return last;
  }

  private:
    Debouncer(): stableTime(0) {}
} Debouncer;
