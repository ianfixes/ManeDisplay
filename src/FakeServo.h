#pragma once

typedef struct Servo {
  int pin;
  int pos;

  void attach(int p) { pin = p; }
  void write(int p) { pos = p; }
} Servo;
