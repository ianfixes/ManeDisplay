#include <ArduinoUnitTests.h>
#include "../src/ManeDisplay.h"


unittest(i2c_address_is_set)
{
  assertEqual(9, SLAVE_I2C_ADDRESS);
}

unittest_main()
