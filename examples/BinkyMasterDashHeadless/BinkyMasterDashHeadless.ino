/**
 * Project Binky master dashboard program -- it just sends pin values to the slave board
 */

#include <Wire.h>
#include <MasterProperties.h>
#include <DashMessage.h>



void setup() {
  Wire.begin(); // I2C bus master -- no ID
  Serial.begin(115200);
}

void loop() {

  const unsigned long t_raw = millis();
  const unsigned long t = t_raw % 10000;
  const bool isBoostCritical = t > 9000;
  const bool isBoostWarning  = !isBoostCritical && (t > 8000);

  Serial.print("t_raw ");
  Serial.print(t_raw);
  Serial.print("\tt ");
  Serial.print(t);
  Serial.print("\t");
  Serial.print(t > 3000 ? "O" : ".");
  Serial.print(t > 4000 ? "O" : ".");
  Serial.print(t > 5000 ? "O" : ".");
  Serial.print(isBoostCritical ? "C" : (isBoostWarning ? "W" : "_"));
  Serial.println();

  DashMessage d;
  d.setBit(MasterSignal::Values::boostWarning,         isBoostWarning);
  d.setBit(MasterSignal::Values::boostCritical,        isBoostCritical);
  d.setBit(MasterSignal::Values::acOn,                 t > 3000);
  d.setBit(MasterSignal::Values::heatedRearWindowOn,   t > 4000);
  d.setBit(MasterSignal::Values::hazardOff,            0);
  d.setBit(MasterSignal::Values::rearFoggerOn,         t > 5000);
  d.setBit(MasterSignal::Values::scrollCAN,            0);
  d.setBit(MasterSignal::Values::scrollPresetColours,  0);
  d.setBit(MasterSignal::Values::scrollRainbowEffects, 0 < (t % 5000) && (t % 5000) < 100);
  d.setBit(MasterSignal::Values::scrollBrightness,     0);

  d.send(Wire, SLAVE_I2C_ADDRESS);

  delay(20); // not sure how often we need to send updates, but ~50 times per second is probably OK
}
