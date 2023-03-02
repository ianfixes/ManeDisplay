# ManeDisplay

Instrument panel driver for a certain horse, with the bother of having to stop all the time to wire bits back on.

## High-Level Purpose

This project displays the electromechanical status of a car to its driver.  Two Arduino boards communicate via I2C: the "master" board, which collects and sends various input data from the hardware, and the "slave" board, which takes additional input and presents all the output to the dashboard panel.


## Installation

Use the [Arduino Library Manager](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries) to "Import a .zip Library", from the latest stable zip file of this project:
https://github.com/ianfixes/ManeDisplay/archive/refs/heads/main.zip

* The "production" sketches are [BinkyMasterDash](examples/BinkyMasterDash/BinkyMasterDash.ino) and [BinkySlaveDash](examples/BinkySlaveDash/BinkySlaveDash.ino), which can be opened from the Arduino IDE's menu.
* Some "debugging" sketches are included:
   * [BinkyMasterDemo](examples/BinkyMasterDemo/BinkyMasterDemo.ino) and [BinkySlaveDemo](examples/BinkySlaveDemo/BinkySlaveDemo.ino), which demonstrate I2C communication between the two boards
   * [optoandservo](examples/optoandservo/optoandservo.ino), which demonstrates the servo configuration
* Some other examples that aided the development of this project are still in there, no guarantees on whether they are current with hardware changes


# Architecture and Configuration

The general input/output diagram is as follows:
```
+--------+   +--------------+   +-------+   +-------------+
| Master |-->| I2C Protocol |-->| Slave |-->| Output Pins |
+--------+   +--------------+   +-------+   +-------------+
   ^                                ^            |
   |                                |            |
   |                                |            v
+------------+           +------------+  +--------------------------+
| Input Pins |           | Input Pins |  | Servo, FastLED, Ignition |
+------------+           +------------+  +--------------------------+


\............./          \........................................../
       :                                      :
Master hardware                         Slave hardware
```


## Software Components

As much logic as possible has been removed from the individual sketches and pushed into the `ManeDisplay` library.  Each file has a distinct purpose and a distinct configuration.


### `CalibratedServo.h` - For defining and operating servos
This file provides two constructs: a `Range` and a `CalibratedServo`.

Without `CalibratedServo`, key aspects of the setup and behavior of the servo are scattered throughout the sketch; the pin in one place, the attachment to the pin in another place, the limits in another place, and the usage in yet another place:
```c++
#include <Servo.h>

Servo myDial;                                     // definition of the servo

void setup() {
  myDial.attach(someOutputPin);                   // setup of the servo - assignment of pin
}

void loop() {
  int tempVal = someValueFromAnalogRead;
  int outputVal = map(tempVal, 0, 1023, 0, 180);  // definition of input and output ranges + usage
  myDial.write(outputVal);                        // usage of the servo
}
```

_With_ `CalibratedServo`, the sections are grouped more logically: definition, setup, and usage.  No opportunities to mismatch the pin attachment or the value mapping.
```c++
#include <CalibratedServo.h>

Range inputRange { 0, 1023 };                                   // definition of the input range
Range outputRange { 0, 180 };                                   // definition of the output range
CalibratedServo myDial(someOutputPin, inputRange, outputRange); // definition of the servo: all in one

void setup() {
  myDial.setup();                                               // setup of the servo
}

void loop() {
  myDial.write(someValueFromAnalogRead);                        // usage of the servo
}
```


### `MasterProperties.h` - for defining the input configuration

This is the file that gives names to the pins and the "signals" of the master board.  So if pin assignments are added or changed, this is where that is reflected.

The pin assignment are defined as follows, so that instead of writing `3` we can write the pin _name_ as `MasterPin::Values::dragChuteDeployed`, or whatever the pin happens to describe:
```c++
namespace MasterPin {
  enum Values {
    dragChuteDeployed      = 3,
    candyBarDispenserEmpty = 4
    // etc
  }
};
```

Similarly, the information on the pin is a type of _signal_, and we need a `0`-based list of what signals are coming from the master board.  These will be a direct copy of the pin names, but in a different namespace:

```c++
namespace MasterSignal {
  enum Values {
    dragChuteDeployed      = 0,
    candyBarDispenserEmpty = 1
    // etc
  }
};
```

### `DashMessage.h` - for defining the wire protocol

This is the file that defines how `MasterSignal` values are packed and unpacked from bytes that flow across I2C.

This file is necessary because more than 8 bits (as of this writing, 10) need to be transmitted, requiring multiple bytes to transmit any message.  One option would be to send 1 bit per message byte, with 7 bits to say what kind of info is being transmitted and 1 bit of information.  That's a lot of overhad.

We can do a bit better by reserving bit positions in each byte for specific signal data, and (knowing the total message length ahead of time) processing a chain of those bytes at once.  All we need to know is where the message starts.

In brief, the wire format is as follows: the MSB of every byte transmitted is reserved for specifying whether this byte is the first of the sequence.  The reader discards received bytes until it finds one with a `1` in the MSB, then reads full messages from that point in the stream.

The bit positions are controlled by the `MasterSignal` enum.

To keep this file up to date, take note of the following properties:

```c++
const uint8_t SLAVE_I2C_ADDRESS = 9;                  // the I2C address that will be used
const unsigned int WIRE_PROTOCOL_MESSAGE_LENGTH = 2;  // divide the number of inputs by 7, and round up

typedef struct DashMessage {

  // set the signal values from their corresponding digital input pins.
  // the spanning of bit positions across multiple messages is handled automatically by setBit.
  void setFromPins(int (*myDigitalRead)(unsigned char)) {
    setBit(MasterSignal::Values::dragChuteDeployed, myDigitalRead(MasterPin::Values::dragChuteDeployed));
    /// and so on
  }
}
```

The struct is generally used by the sender as follows

```c++
  DashMessage d(digitalRead); // create a message from the current state of pins
  d.send(Wire, SLAVE_I2C_ADDRESS);
```

And by the receiver as follows:
```c++
  // I2C receiver function
  void receiveDashMessage(int /* bytes */) {
    DashMessage dm;
    // this will consume all available messages in order
    while (Wire.available() >= (int)WIRE_PROTOCOL_MESSAGE_LENGTH) {
      dm.setFromWire(Wire);   // create a message from the wire
      if (!dm.isError()) {    // check decoding status
        dm.getBit(MasterSignal::Values::dragChuteDeployed); // access the data
      }
    }
  }

  // (need to register the receiver function)
  void setup() {
    Wire.begin(SLAVE_I2C_ADDRESS);      // Start the I2C Bus as Slave on address
    Wire.onReceive(receiveDashMessage); // Attach a function to trigger when something is received.
  }
```

### `SlaveProperties.h` - for defining input and output configuration

This is the file that gives names to the pins and the "signals" of the slave board -- it translates the physical state of the input data to a software object.  So if pin assignments are added or changed, this is where that is reflected.  It also wraps a DashMessage object to carry the received data from the master.

```c++
namespace SlavePin {
  enum Values {
    hoonometerInput  = A3,
    hoonServo        = 6,
    kettleOn         = 8,
    /// etc
  };
}
```


The following sections of `SlaveState` should be updated accordingly as pins or signals change (note that no corresponding struct exists for the master, because that information is stored in DashMessage structures):
```c++
typedef struct SlaveState {
  bool kettleIsOn;
  int hoonLevel;
  /// etc

  static void setup(void (*myPinMode)(pin_size_t, int)) {
    myPinMode(SlavePin::Values::hoonometerInput, INPUT);
    myPinMode(SlavePin::Values::kettleOn,        INPUT);
    /// etc
  }

  void setFromPins(int (*myDigitalRead)(unsigned char), int (*myAnalogRead)(unsigned char)) {
    kettleIsOn = myDigitalRead(SlavePin::Values::kettleOn);

    hoonLevel  = myAnalogRead(SlavePin::Values::hoonometerInput);

    /// etc
  }

  /// snip

}
```

The state can be read simply:

```c++
  SlaveState state(digitalRead, analogRead);  // yes, the functions themselves are passed as arguments.
  state.kettleIsOn; // access a value
  state.getMasterSignal(MasterPin::Values::dragChuteDeployed); // access a value from the message
```


### `LEDState.h` - All LED behaviors

This file is complex enough that its documentation is contained throughout its source.  The short description is that a set of state classes are defined (what to do, when to stop doing it), and a set of state machines classes include some of those states as members, as needed.  The state machines define how to choose the next state when the current state is done.

The most user-configurable portion of the LEDState is at the top: the definition of the colors for the LEDs, in case these need to be changed.  (These are expressed in `FastLED` format, which can be a named constant or a raw HTML color code expressed in hex as a 32-bit integer.):

```c++
const int FLASH_DURATION_MS = 100;  // what we want for all flashing LEDs. this is 1/2 of the flash

const struct CRGB COLOR_BLACK  = CRGB::HTMLColorCode(CRGB::Black);
const struct CRGB COLOR_WHITE  = CRGB::HTMLColorCode(CRGB::White);
const struct CRGB COLOR_RED    = CRGB::HTMLColorCode(CRGB::Red);
const struct CRGB COLOR_YELLOW = CRGB::HTMLColorCode(CRGB::Yellow);
const struct CRGB COLOR_BLUE   = CRGB::HTMLColorCode(CRGB::Blue);
const struct CRGB COLOR_AMBER  = CRGB((uint32_t)0xFFBF00);
```

Here's how we would define an LED state machine for one of the inputs named above:

```c++
// inherit from SimpleLED. SimpleLED handles rainbow mode for us as well as a solid color.
// the control of the solid indicator can be delegated to child classes like the one we define here.
class DragChuteLED : public SimpleLED {
public:
  // delegate the constructor to the parent class, but fill in our desired color
  DragChuteLED(struct CRGB* leds, int numLEDs, int index) : SimpleLED(leds, numLEDs, index, COLOR_RED) {}

  // override the isOn criteria to respond to the specific signal this LED represents
  virtual bool isOn(unsigned long millis, const SlaveState &slave) override {
    return slave.getMasterSignal(MasterSignal::Values::dragChuteDeployed);
  }
};
```


### `DashState.h` - All the indicator definitions

This file defines the physical layout of the dashboard indicator panel.  Any changes to the physical panel or the properties of its indicators should be reflected in this file.

See [BinkySlaveDash](examples/BinkySlaveDash/BinkySlaveDash.ino) for the usage.

The DashState file has 5 main sections:
1. declarations of constants
2. declarations of variables for the attached hardware (Servos, FastLEDs) in the `DashState` struct
3. initialization of the `DashState` struct (delegating constructors or within the constructor)
4. a `setup()` function
5. an `apply()` function that runs during a loop.  (We don't use "loop" because the application of the state comes from both synchronous and asynchronous sources.)

Here's how hardware is defined in those sections:
#### Servos
```c++
// 1. declaration of constants
const Range hoonLevelLimit { 0, 1023 };  // input expected from the analog read
const Range hoonDialLimit  { 10, 170 };   // output range of the dial

// 2. declaration of variables
typedef struct DashState {
  CalibratedServo hoonGauge;

   /// etc

// 3. initialization of the DashState
  DashState(DashSupport ds):
    support(ds),
    hoonGauge(SlavePin::Values::hoonServo, hoonLevelLimit, hoonDialLimit)
  {}

// 4. setup()
  void setup() {
    hoonGauge.setup();
  }

// 5. apply()
  void apply(unsigned long nMillis) {
    hoonGauge.write(nextState.hoonLevel);
  }

}
```


#### LEDs
```c++
// 1. declaration of constants
const Range LEDStripBrightnessLimit { 5, 255 };

// LED assignments across the dash. numbers favor left to right reading
namespace DashLED {
  enum Values {
    dragChute           = 15,
    /// etc
  };
}

// 2. declaration of variables
typedef struct DashState {
  StatefulLED* statefulLeds[NUM_DASH_LEDS] = {
    new DragChuteLED(leds, NUM_DASH_LEDS, DashLED::Values::dragChute),
    /// etc
  }

// 3. initialization of the DashState: N/A

// 4. setup(): N/A - FastLED is already being set up, no code change required

// 5. apply(): N/A - LEDState behaviors already set up and running
}
```
