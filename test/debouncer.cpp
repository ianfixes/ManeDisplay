#include <ArduinoUnitTests.h>
#include "../src/Debouncer.h"

unittest(debounce_stay_false)
{
  Debouncer d(3);

  assertEqual(Debouncer::Event::none, d.eventOf(100, false));
  assertEqual(Debouncer::Event::none, d.eventOf(101, false));
  assertEqual(Debouncer::Event::none, d.eventOf(102, false));
  assertEqual(Debouncer::Event::none, d.eventOf(103, false));
  assertEqual(Debouncer::Event::none, d.eventOf(104, false));
  assertEqual(Debouncer::Event::none, d.eventOf(105, false));
}

unittest(debounce_turn_true_fast_polling)
{
  Debouncer d1(3);

  assertEqual(Debouncer::Event::none,   d1.eventOf(100, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(101, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(102, true));
  assertEqual(Debouncer::Event::toHigh, d1.eventOf(103, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(104, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(114, true));
}

unittest(debounce_turn_true_slow_polling)
{
  Debouncer d1(3);

  assertEqual(Debouncer::Event::none,   d1.eventOf(100, true));
  assertEqual(Debouncer::Event::toHigh, d1.eventOf(110, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(111, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(121, true));
}

unittest(debounce_return_false_fast_polling)
{
  Debouncer d1(3);

  assertEqual(Debouncer::Event::none,   d1.eventOf(100, true));
  assertEqual(Debouncer::Event::toHigh, d1.eventOf(110, true));

  assertEqual(Debouncer::Event::none,   d1.eventOf(200, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(201, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(202, false));
  assertEqual(Debouncer::Event::toLow,  d1.eventOf(203, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(204, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(214, false));
}

unittest(debounce_return_false_slow_polling)
{
  Debouncer d1(3);
  assertEqual(Debouncer::Event::none,   d1.eventOf(100, true));
  assertEqual(Debouncer::Event::toHigh, d1.eventOf(110, true));

  assertEqual(Debouncer::Event::none,   d1.eventOf(200, false));
  assertEqual(Debouncer::Event::toLow,  d1.eventOf(210, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(211, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(221, false));
}

unittest(bouncing_true)
{
  Debouncer d1(3);
  assertEqual(Debouncer::Event::none,   d1.eventOf(100, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(101, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(102, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(103, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(104, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(105, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(105, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(106, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(107, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(108, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(109, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(110, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(111, false));
  assertEqual(Debouncer::Event::none,   d1.eventOf(112, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(113, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(114, true));
  assertEqual(Debouncer::Event::toHigh, d1.eventOf(115, true));
  assertEqual(Debouncer::Event::none,   d1.eventOf(111, false));
}



unittest_main()
