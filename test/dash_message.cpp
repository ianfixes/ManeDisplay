#include <ArduinoUnitTests.h>
#include <Wire.h>

// TODO: need to define this in arduino_ci
typedef unsigned char pin_size_t;
#include "../src/DashMessage.h"


// a fake digital reader
uint16_t fakePins = 0b0000000000000000;
int fakeDigitalRead(unsigned char pin) {
  uint16_t mask = 0b0000000000000001 << pin;
  return fakePins & mask;
}


unittest(i2c_address_is_set)
{
  assertEqual(9, SLAVE_I2C_ADDRESS);
}

unittest(wire_protocol_default_constructor)
{
  // a default message is "no error" with all bits false
  DashMessage d;
  assertFalse(d.isError());
  assertEqual((unsigned int)FIRST_FRAME_MARKER_MASK, (unsigned int) d.rawData[0]);
  assertEqual(0, (unsigned int) d.rawData[1]);

  assertFalse(d.getBit(MasterSignal::Values::boostWarning));
  assertFalse(d.getBit(MasterSignal::Values::boostCritical));
  assertFalse(d.getBit(MasterSignal::Values::acOn));
  assertFalse(d.getBit(MasterSignal::Values::heatedRearWindowOn));
  assertFalse(d.getBit(MasterSignal::Values::hazardOff));
  assertFalse(d.getBit(MasterSignal::Values::rearFoggerOn));
  assertFalse(d.getBit(MasterSignal::Values::scrollCAN));
  assertFalse(d.getBit(MasterSignal::Values::scrollPresetColours));
  assertFalse(d.getBit(MasterSignal::Values::scrollRainbowEffects));
  assertFalse(d.getBit(MasterSignal::Values::scrollBrightness));
}

unittest(wire_protocol_array_constructor)
{
  // if we construct via the array constructor, data will be sanitized
  byte payload[2] = {
    0b00000000,
    0b00000000
  };
  DashMessage d(payload); // constructor always makes valid payloads by setting the MSB appropriately
  assertFalse(d.isError());
  assertEqual((unsigned int)FIRST_FRAME_MARKER_MASK, (unsigned int) d.rawData[0]);
  assertEqual(0, (unsigned int) d.rawData[1]);

  // we can override the sanitization behavior and force an error
  d.setRawBytes(payload);
  assertTrue(d.isError());
  assertEqual(0, (unsigned int) d.rawData[0]);
  assertEqual(0, (unsigned int) d.rawData[1]);
}

unittest(wire_protocol_errors)
{
  // test that the set/clear functions produce the expected change in the isError() method
  DashMessage d;
  assertFalse(d.isError());
  d.setError();
  assertTrue(d.isError());
  d.clearError();
  assertFalse(d.isError());
}


unittest(wire_protocol_bit_positions_false)
{
  // sanity check that we can force all bits to report false
  byte payload[2] = {
    0b00000000,
    0b00000000
  };
  DashMessage d(payload);
  assertFalse(d.getBit(MasterSignal::Values::boostWarning));
  assertFalse(d.getBit(MasterSignal::Values::boostCritical));
  assertFalse(d.getBit(MasterSignal::Values::acOn));
  assertFalse(d.getBit(MasterSignal::Values::heatedRearWindowOn));
  assertFalse(d.getBit(MasterSignal::Values::hazardOff));
  assertFalse(d.getBit(MasterSignal::Values::rearFoggerOn));
  assertFalse(d.getBit(MasterSignal::Values::scrollCAN));
  assertFalse(d.getBit(MasterSignal::Values::scrollPresetColours));
  assertFalse(d.getBit(MasterSignal::Values::scrollRainbowEffects));
  assertFalse(d.getBit(MasterSignal::Values::scrollBrightness));
}

unittest(wire_protocol_bit_positions_true)
{
  // sanity check that we can force all bits to report true
  byte payload[2] = {
    0b01111111,
    0b00000111
  };
  DashMessage d(payload);
  assertTrue(d.getBit(MasterSignal::Values::boostWarning));
  assertTrue(d.getBit(MasterSignal::Values::boostCritical));
  assertTrue(d.getBit(MasterSignal::Values::acOn));
  assertTrue(d.getBit(MasterSignal::Values::heatedRearWindowOn));
  assertTrue(d.getBit(MasterSignal::Values::hazardOff));
  assertTrue(d.getBit(MasterSignal::Values::rearFoggerOn));
  assertTrue(d.getBit(MasterSignal::Values::scrollCAN));
  assertTrue(d.getBit(MasterSignal::Values::scrollPresetColours));
  assertTrue(d.getBit(MasterSignal::Values::scrollRainbowEffects));
  assertTrue(d.getBit(MasterSignal::Values::scrollBrightness));
}

unittest(wire_protocol_bit_positions_individual)
{
  // sanity check that we can set one bit at a time and we get the one we expect.
  // note that we use the array-of-booleans initialization method
  unsigned int flags_len = MASTERSIGNAL_MAX + 1;
  bool flags[flags_len];

  for (int i = MASTERSIGNAL_MIN; i <= MASTERSIGNAL_MAX; ++i) {
    // only one flag at a time should be true
    for (int j = MASTERSIGNAL_MIN; j <= MASTERSIGNAL_MAX; ++j) flags[j] = i == j;

    DashMessage d(flags, flags_len);
    assertFalse(d.isError());
    // assertEqual(0, (unsigned int) d.rawData[0] - FIRST_FRAME_MARKER_MASK);
    // assertEqual(0, (unsigned int) d.rawData[1]);
    assertEqual(i == MasterSignal::Values::boostWarning,         d.getBit(MasterSignal::Values::boostWarning));
    assertEqual(i == MasterSignal::Values::boostCritical,        d.getBit(MasterSignal::Values::boostCritical));
    assertEqual(i == MasterSignal::Values::acOn,                 d.getBit(MasterSignal::Values::acOn));
    assertEqual(i == MasterSignal::Values::heatedRearWindowOn,   d.getBit(MasterSignal::Values::heatedRearWindowOn));
    assertEqual(i == MasterSignal::Values::hazardOff,            d.getBit(MasterSignal::Values::hazardOff));
    assertEqual(i == MasterSignal::Values::rearFoggerOn,         d.getBit(MasterSignal::Values::rearFoggerOn));
    assertEqual(i == MasterSignal::Values::scrollCAN,            d.getBit(MasterSignal::Values::scrollCAN));
    assertEqual(i == MasterSignal::Values::scrollPresetColours,  d.getBit(MasterSignal::Values::scrollPresetColours));
    assertEqual(i == MasterSignal::Values::scrollRainbowEffects, d.getBit(MasterSignal::Values::scrollRainbowEffects));
    assertEqual(i == MasterSignal::Values::scrollBrightness,     d.getBit(MasterSignal::Values::scrollBrightness));
  }
}

unittest(wire_protocol_bit_positions_set_get)
{
  // this tests the set/get functions that manipulate a DashMessage in-place
  DashMessage d;
  for (int i = MASTERSIGNAL_MIN; i <= MASTERSIGNAL_MAX; ++i) {
    // only one flag at a time should be true
    d.setBit((MasterSignal::Values)i, true);
    assertFalse(d.isError());

    assertEqual(i == MasterSignal::Values::boostWarning,         d.getBit(MasterSignal::Values::boostWarning));
    assertEqual(i == MasterSignal::Values::boostCritical,        d.getBit(MasterSignal::Values::boostCritical));
    assertEqual(i == MasterSignal::Values::acOn,                 d.getBit(MasterSignal::Values::acOn));
    assertEqual(i == MasterSignal::Values::heatedRearWindowOn,   d.getBit(MasterSignal::Values::heatedRearWindowOn));
    assertEqual(i == MasterSignal::Values::hazardOff,            d.getBit(MasterSignal::Values::hazardOff));
    assertEqual(i == MasterSignal::Values::rearFoggerOn,         d.getBit(MasterSignal::Values::rearFoggerOn));
    assertEqual(i == MasterSignal::Values::scrollCAN,            d.getBit(MasterSignal::Values::scrollCAN));
    assertEqual(i == MasterSignal::Values::scrollPresetColours,  d.getBit(MasterSignal::Values::scrollPresetColours));
    assertEqual(i == MasterSignal::Values::scrollRainbowEffects, d.getBit(MasterSignal::Values::scrollRainbowEffects));
    assertEqual(i == MasterSignal::Values::scrollBrightness,     d.getBit(MasterSignal::Values::scrollBrightness));

    d.setBit((MasterSignal::Values)i, false);
  }

}

unittest(payload_from_digitalread)
{
  // validate that we can read directly from the pins (or a mock function)
  fakePins = 0b0101010101010101;
  DashMessage d(fakeDigitalRead);

  assertEqual(MasterSignal::Values::boostWarning         % 2, d.getBit(MasterSignal::Values::boostWarning));
  assertEqual(MasterSignal::Values::boostCritical        % 2, d.getBit(MasterSignal::Values::boostCritical));
  assertEqual(MasterSignal::Values::acOn                 % 2, d.getBit(MasterSignal::Values::acOn));
  assertEqual(MasterSignal::Values::heatedRearWindowOn   % 2, d.getBit(MasterSignal::Values::heatedRearWindowOn));
  assertEqual(MasterSignal::Values::hazardOff            % 2, d.getBit(MasterSignal::Values::hazardOff));
  assertEqual(MasterSignal::Values::rearFoggerOn         % 2, d.getBit(MasterSignal::Values::rearFoggerOn));
  assertEqual(MasterSignal::Values::scrollCAN            % 2, d.getBit(MasterSignal::Values::scrollCAN));
  assertEqual(MasterSignal::Values::scrollPresetColours  % 2, d.getBit(MasterSignal::Values::scrollPresetColours));
  assertEqual(MasterSignal::Values::scrollRainbowEffects % 2, d.getBit(MasterSignal::Values::scrollRainbowEffects));
  assertEqual(MasterSignal::Values::scrollBrightness     % 2, d.getBit(MasterSignal::Values::scrollBrightness));
}


unittest(dashmessage_assignment)
{
  // validate that the assignment operator works)
  fakePins = 0b0101010101010101;
  DashMessage a(fakeDigitalRead);
  DashMessage d = a;

  assertEqual(MasterSignal::Values::boostWarning         % 2, d.getBit(MasterSignal::Values::boostWarning));
  assertEqual(MasterSignal::Values::boostCritical        % 2, d.getBit(MasterSignal::Values::boostCritical));
  assertEqual(MasterSignal::Values::acOn                 % 2, d.getBit(MasterSignal::Values::acOn));
  assertEqual(MasterSignal::Values::heatedRearWindowOn   % 2, d.getBit(MasterSignal::Values::heatedRearWindowOn));
  assertEqual(MasterSignal::Values::hazardOff            % 2, d.getBit(MasterSignal::Values::hazardOff));
  assertEqual(MasterSignal::Values::rearFoggerOn         % 2, d.getBit(MasterSignal::Values::rearFoggerOn));
  assertEqual(MasterSignal::Values::scrollCAN            % 2, d.getBit(MasterSignal::Values::scrollCAN));
  assertEqual(MasterSignal::Values::scrollPresetColours  % 2, d.getBit(MasterSignal::Values::scrollPresetColours));
  assertEqual(MasterSignal::Values::scrollRainbowEffects % 2, d.getBit(MasterSignal::Values::scrollRainbowEffects));
  assertEqual(MasterSignal::Values::scrollBrightness     % 2, d.getBit(MasterSignal::Values::scrollBrightness));
}


unittest(sent_payload)
{
  // validate that we can send a message onto the wire
  const int addr = 7;
  fakePins = 0b0001010101010101;
           //     ^ ^ ^ ^ ^ xxx   ... is how these map to the enum values
           //     ===------
           //      5    42
  DashMessage d(fakeDigitalRead);
  assertFalse(d.isError());
  assertTrue( d.getBit(MasterSignal::Values::boostCritical));
  assertFalse(d.getBit(MasterSignal::Values::acOn));
  assertTrue( d.getBit(MasterSignal::Values::heatedRearWindowOn));
  assertFalse(d.getBit(MasterSignal::Values::hazardOff));
  assertTrue( d.getBit(MasterSignal::Values::rearFoggerOn));
  assertFalse(d.getBit(MasterSignal::Values::scrollCAN));
  assertTrue( d.getBit(MasterSignal::Values::scrollPresetColours));
  assertFalse(d.getBit(MasterSignal::Values::scrollRainbowEffects));
  assertTrue( d.getBit(MasterSignal::Values::scrollBrightness));

  Wire.resetMocks();
  deque<uint8_t>* mosi = Wire.getMosi(addr);  // master out, slave in buffer
  assertEqual(0, mosi->size());
  Wire.begin();

  d.send(Wire, addr);

  assertEqual(2, mosi->size());
  assertEqual(42, (int)mosi->front() - FIRST_FRAME_MARKER_MASK);  // note the frame marker
  mosi->pop_front();
  assertEqual(5, (int)mosi->front());
  mosi->pop_front();
}


unittest(received_good_payload)
{
  //validate that we can receive data from I2C and have it return what we expect
  const byte fakePayload[] = { 42 + FIRST_FRAME_MARKER_MASK, 5 };
  Wire.resetMocks();

  // mocks don't support onReceive, so pretend we just asked a slave device for data
  const int fakeSlaveAddr = 7;
  Wire.begin();
  deque<uint8_t>* miso;
  miso = Wire.getMiso(fakeSlaveAddr);
  miso->push_back(fakePayload[0]);
  miso->push_back(fakePayload[1]);
  assertEqual(2, Wire.requestFrom(fakeSlaveAddr, 2));

  // assert readiness
  assertEqual(2, Wire.available());

  DashMessage d(Wire);
  assertFalse(d.isError());
  assertEqual((unsigned int) fakePayload[0], (unsigned int) d.rawData[0]);
  assertEqual((unsigned int) fakePayload[1], (unsigned int) d.rawData[1]);
  assertTrue( d.getBit(MasterSignal::Values::boostCritical));
  assertFalse(d.getBit(MasterSignal::Values::acOn));
  assertTrue( d.getBit(MasterSignal::Values::heatedRearWindowOn));
  assertFalse(d.getBit(MasterSignal::Values::hazardOff));
  assertTrue( d.getBit(MasterSignal::Values::rearFoggerOn));
  assertFalse(d.getBit(MasterSignal::Values::scrollCAN));
  assertTrue( d.getBit(MasterSignal::Values::scrollPresetColours));
  assertFalse(d.getBit(MasterSignal::Values::scrollRainbowEffects));
  assertTrue( d.getBit(MasterSignal::Values::scrollBrightness));
}

unittest(received_unsynced_payload)
{
  //validate that we can receive data from I2C and have it return what we expect
  const byte fakePayload[] = { 42, 42 + FIRST_FRAME_MARKER_MASK, 5 };
  Wire.resetMocks();

  // mocks don't support onReceive, so pretend we just asked a slave device for data
  const int fakeSlaveAddr = 7;
  Wire.begin();
  deque<uint8_t>* miso;
  miso = Wire.getMiso(fakeSlaveAddr);
  miso->push_back(fakePayload[0]);
  miso->push_back(fakePayload[1]);
  miso->push_back(fakePayload[2]);
  assertEqual(3, Wire.requestFrom(fakeSlaveAddr, 3));

  // assert readiness
  assertEqual(3, Wire.available());

  // assert the message is erroneous
  DashMessage d2(Wire);
  assertTrue(d2.isError());
  assertEqual(2, Wire.available()); // but we should still have data in the buffer

  // now it should work fine
  DashMessage d(Wire);
  assertFalse(d.isError());
  assertEqual((unsigned int) fakePayload[1], (unsigned int) d.rawData[0]);
  assertEqual((unsigned int) fakePayload[2], (unsigned int) d.rawData[1]);
  assertTrue( d.getBit(MasterSignal::Values::boostCritical));
  assertFalse(d.getBit(MasterSignal::Values::acOn));
  assertTrue( d.getBit(MasterSignal::Values::heatedRearWindowOn));
  assertFalse(d.getBit(MasterSignal::Values::hazardOff));
  assertTrue( d.getBit(MasterSignal::Values::rearFoggerOn));
  assertFalse(d.getBit(MasterSignal::Values::scrollCAN));
  assertTrue( d.getBit(MasterSignal::Values::scrollPresetColours));
  assertFalse(d.getBit(MasterSignal::Values::scrollRainbowEffects));
  assertTrue( d.getBit(MasterSignal::Values::scrollBrightness));
}


unittest_main()
