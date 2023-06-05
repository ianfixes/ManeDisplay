#include <ArduinoUnitTests.h>
#include "../src/LedState.h"

unittest(shimmer_distance_velocity_function)
{
  ShimmerState s({0, 0}, 6500, 0.025, 8, -2000, -1);
  assertEqual(  0, s.velVsDistance(20000));
  assertEqual(  0, s.velVsDistance(10000));
  assertEqual(  0, s.velVsDistance(5000));
  assertEqual(  0, s.velVsDistance(2000));
  assertEqual(  0, s.velVsDistance(1000));
  assertEqual( 98, s.velVsDistance(500));
  assertEqual(248, s.velVsDistance(100));
  assertEqual(253, s.velVsDistance(50));
  assertEqual(254, s.velVsDistance(20));
  assertEqual(254, s.velVsDistance(10));
  assertEqual(254, s.velVsDistance(5));
  assertEqual(254, s.velVsDistance(2));
  assertEqual(254, s.velVsDistance(1));
  assertEqual(255, s.velVsDistance(0));
}

unittest(shimmer_distance_to_line)
{
  ShimmerState s1({0, 0}, 20001, 0, 1, 0, 0);
  s1.m_activationTimeMs = 0;
  assertEqual(20000, s1.distanceToLine(20000));
  assertEqual(10000, s1.distanceToLine(10000));
  assertEqual( 5000, s1.distanceToLine(5000));
  assertEqual( 2000, s1.distanceToLine(2000));
  assertEqual( 1000, s1.distanceToLine(1000));
  assertEqual(  500, s1.distanceToLine(500));
  assertEqual(  100, s1.distanceToLine(100));
  assertEqual(   50, s1.distanceToLine(50));
  assertEqual(   20, s1.distanceToLine(20));
  assertEqual(   10, s1.distanceToLine(10));
  assertEqual(    5, s1.distanceToLine(5));
  assertEqual(    2, s1.distanceToLine(2));
  assertEqual(    1, s1.distanceToLine(1));
  assertEqual(    0, s1.distanceToLine(0));

  ShimmerState s2({0, 0}, 20001, 0, 1, 1000, 0);
  s2.m_activationTimeMs = 0;
  assertEqual(21000, s2.distanceToLine(20000));
  assertEqual(11000, s2.distanceToLine(10000));
  assertEqual( 6000, s2.distanceToLine(5000));
  assertEqual( 3000, s2.distanceToLine(2000));
  assertEqual( 2000, s2.distanceToLine(1000));
  assertEqual( 1500, s2.distanceToLine(500));
  assertEqual( 1100, s2.distanceToLine(100));
  assertEqual( 1050, s2.distanceToLine(50));
  assertEqual( 1020, s2.distanceToLine(20));
  assertEqual( 1010, s2.distanceToLine(10));
  assertEqual( 1005, s2.distanceToLine(5));
  assertEqual( 1002, s2.distanceToLine(2));
  assertEqual( 1001, s2.distanceToLine(1));
  assertEqual( 1000, s2.distanceToLine(0));

  // with a slope of -1, this should approximate the square root of 2
  ShimmerState s3({0, 0}, 20001, 0, 1, 0, -1);
  s3.m_activationTimeMs = 0;
  assertEqual(14142, s3.distanceToLine(20000));
  assertEqual( 7071, s3.distanceToLine(10000));
  assertEqual( 3535, s3.distanceToLine(5000));
  assertEqual( 1414, s3.distanceToLine(2000));
  assertEqual(  707, s3.distanceToLine(1000));
  assertEqual(  353, s3.distanceToLine(500));
  assertEqual(   70, s3.distanceToLine(100));
  assertEqual(   35, s3.distanceToLine(50));
  assertEqual(   14, s3.distanceToLine(20));
  assertEqual(    7, s3.distanceToLine(10));
  assertEqual(    3, s3.distanceToLine(5));
  assertEqual(    1, s3.distanceToLine(2));
  assertEqual(    0, s3.distanceToLine(1));
  assertEqual(    0, s3.distanceToLine(0));

  ShimmerState s4({0, 0}, 6500, 0, 1, -2000, -1);
  s4.m_activationTimeMs = 0;
  assertEqual(1060, s4.distanceToLine(20000));
  assertEqual(1060, s4.distanceToLine(10000));
  assertEqual(2121, s4.distanceToLine(5000));
  assertEqual(   0, s4.distanceToLine(2000));
  assertEqual( 707, s4.distanceToLine(1000));
  assertEqual(1060, s4.distanceToLine(500));
  assertEqual(1343, s4.distanceToLine(100));
  assertEqual(1378, s4.distanceToLine(50));
  assertEqual(1400, s4.distanceToLine(20));
  assertEqual(1407, s4.distanceToLine(10));
  assertEqual(1410, s4.distanceToLine(5));
  assertEqual(1412, s4.distanceToLine(2));
  assertEqual(1413, s4.distanceToLine(1));
  assertEqual(1414, s4.distanceToLine(0));
}

unittest(shimmer_animation_postion)
{
  ShimmerState s({0, 0}, 6500, 0.025, 8, -2000, -1);
  s.m_activationTimeMs = 0;
  assertEqual(    0, s.animationPosition(6500));
  assertEqual(51992, s.animationPosition(6499));
  assertEqual(40000, s.animationPosition(5000));
  assertEqual(16000, s.animationPosition(2000));
  assertEqual( 8000, s.animationPosition(1000));
  assertEqual( 4000, s.animationPosition(500));
  assertEqual(  800, s.animationPosition(100));
  assertEqual(  400, s.animationPosition(50));
  assertEqual(  160, s.animationPosition(20));
  assertEqual(   80, s.animationPosition(10));
  assertEqual(   40, s.animationPosition(5));
  assertEqual(   16, s.animationPosition(2));
  assertEqual(    8, s.animationPosition(1));
  assertEqual(    0, s.animationPosition(0));
}




unittest_main()
