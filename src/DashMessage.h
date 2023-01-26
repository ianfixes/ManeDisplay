#pragma once

#include <Arduino.h>
#include <Wire.h>

const uint8_t SLAVE_I2C_ADDRESS = 9;

/*

Defining a wire protocol here

At the moment we are just trying to convey raw bits from master
to slave device.  So we define a simple framing structure where
the first bit of every byte indicates "this is the first byte" -
'1' for the first byte and '0' for as many others as we need.

1st byte   2nd byte   3rd byte (and so on)
1xxx xxxx  0xxx xxxx  0xxx xxxx

The receiver will discard bytes until it finds one with the
leading '1', then begin its normal processing
*/

// the number of bytes needed for our contrived protocol
const unsigned int WIRE_PROTOCOL_MESSAGE_LENGTH = 2;

// bit positions in our protocol. these exceed 8 bits, so they aren't
// compatible with bit positions in a long int
enum MasterSignal {
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

// min and max for the enum, for iterating
const unsigned int MASTERSIGNAL_MIN = MasterSignal::led23to100pctAmber;
const unsigned int MASTERSIGNAL_MAX = MasterSignal::scrollBrightness;

// the way that we will indicate the first byte in the protocol sequence
const byte FIRST_FRAME_MARKER_MASK = 0b10000000;


/**

  This struct defines all the bit accesses, creation, reading,
  and sending for our protocol.

 */
typedef struct DashMessage {
  byte rawData[WIRE_PROTOCOL_MESSAGE_LENGTH]; // the underlying data

  // extract a single bit
  inline bool getBit(MasterSignal position) {
    return rawData[position / 7] & (0b00000001 << position % 7);
  }

  // set a single bit
  inline void setBit(MasterSignal position, bool val) {
    const byte mask = 0b00000001 << (position % 7);
    if (val)
      rawData[position / 7] |= mask;
    else
      rawData[position / 7] &= ~mask;
  }

  // set an entire byte unconditionally
  inline void setRawByte(unsigned int position, byte val) {
    rawData[position] = val;
  }

  // set an entire byte but obey framing
  inline void setByte(unsigned int position, byte val) {
    setRawByte(position, position ? (val & ~FIRST_FRAME_MARKER_MASK) : (val | FIRST_FRAME_MARKER_MASK));
  }

  // set a message unconditionally
  inline void setRawBytes(byte* data) {
    for (unsigned int i = 0; i < WIRE_PROTOCOL_MESSAGE_LENGTH; ++i) setRawByte(i, data[i]);
  }

  // set a message but obey framing
  inline void setBytes(byte* data) {
    for (unsigned int i = 0; i < WIRE_PROTOCOL_MESSAGE_LENGTH; ++i) setByte(i, data[i]);
  }

  // if frame marker is unset, this is an error
  inline bool isError() {
    return !(rawData[0] & FIRST_FRAME_MARKER_MASK);
  }

  // bad framing indicates an error, so cause this
  inline void setError() {
    rawData[0] &= ~FIRST_FRAME_MARKER_MASK;
  }

  // bad framing indicates an error, so cause this
  inline void clearError() {
    rawData[0] |= FIRST_FRAME_MARKER_MASK;
  }

  // empty valid data set
  inline void initFrames() {
    for (unsigned int i = 0; i < WIRE_PROTOCOL_MESSAGE_LENGTH; ++i) setByte(i, 0);
  }

  // read payload from digital input pins
  void setFromPins(int (*myDigitalRead)(unsigned char)) {
    setBit(MasterSignal::led23to100pctAmber,   myDigitalRead(3));
    setBit(MasterSignal::led23to100pctRed,     myDigitalRead(4));
    setBit(MasterSignal::led24to100pctBlue,    myDigitalRead(5));
    setBit(MasterSignal::led25to100pctYellow,  myDigitalRead(6));
    setBit(MasterSignal::led27toBlack,         myDigitalRead(7));
    setBit(MasterSignal::led26toAmber,         myDigitalRead(8));
    setBit(MasterSignal::d3Low,                myDigitalRead(9));
    setBit(MasterSignal::scrollPresetColours,  myDigitalRead(10));
    setBit(MasterSignal::scrollRainbowEffects, myDigitalRead(11));
    setBit(MasterSignal::scrollBrightness,     myDigitalRead(12));
  }

  // read input from I2C
  void setFromWire(TwoWire wire) {
    if (wire.available() < (int)WIRE_PROTOCOL_MESSAGE_LENGTH) {
      setError();
    } else {
      clearError();
      for (unsigned int i = 0; i < WIRE_PROTOCOL_MESSAGE_LENGTH; ++i) {
        setRawByte(i, wire.read());
        // verify framing, return errored item if not
        if (!!i == !!(rawData[i] & FIRST_FRAME_MARKER_MASK)) {
          setError();
          return; // leave the remaining bytes
        }
      }
    }
  }

  // construct empty container
  DashMessage() {
    initFrames();
  }

  // construct from byte array
  DashMessage(byte* data) {
    setBytes(data);
  }

  // pack an array of booleans into the raw data bytes
  DashMessage(bool* data, unsigned int len) {
    initFrames();
    for (unsigned int i = 0; i < (WIRE_PROTOCOL_MESSAGE_LENGTH * 7) && i < len; ++i) {
      setBit((MasterSignal)i, data[i]);
    }
  }

  // construct from arduino inputs
  DashMessage(int (*myDigitalRead)(unsigned char)) {
    initFrames();
    setFromPins(myDigitalRead);
  }

  // construct from the wire
  DashMessage(TwoWire wire) {
    setFromWire(wire);
  }

  // send on the wire
  void send(TwoWire &wire, int destinationAddress) {
    wire.beginTransmission(destinationAddress);
    for (unsigned int i = 0; i < WIRE_PROTOCOL_MESSAGE_LENGTH; ++i) wire.write((uint8_t)rawData[i]);
    wire.endTransmission();
  }


} DashMessage;
