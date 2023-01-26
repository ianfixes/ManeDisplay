/**
 * Troubleshooting / debugging program for the Binky instrument panel.
 *
 * This will setup several behaviors and run one slice of each behavior on every loop()
 *
 * 1. Flip the D3 pin on an interval
 * 2. Flash the onboard LED according to input from the I2C master
 *   Based on https://create.arduino.cc/projecthub/PIYUSH_K_SINGH/master-slave-i2c-connection-f1aa53
 * 3. Cycle the dash LEDs through a brightness and color cycle
 */

#include <Wire.h>
#include <FastLED.h>
#include <DashMessage.h>



/////////  a behavior to flip the D3 pin on some interval in milliseconds, for testing

const int d3pin          = 3;
const int d3FlipInterval = 1000;

void d3Setup() {
  pinMode(d3pin, OUTPUT);
}

void d3Loop(unsigned long nMillis) {
  int ledState = ((nMillis % (d3FlipInterval * 2)) > d3FlipInterval) ? HIGH : LOW;
  digitalWrite(d3pin, ledState);
}




/////////// A behavior to flash the onboard LED according to what is received from the I2C master

int morseState = 0;

void receiveMorse(int bytes) {
  // we expect to get only one byte at a time, but in case there is a buildup just
  //  clear it out and keep the last sent value
  for (int i = 0; i < bytes; ++i) {
    morseState = Wire.read();
  }
}

void morseSetup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin(SLAVE_I2C_ADDRESS);  // Start the I2C Bus as Slave on address
  Wire.onReceive(receiveMorse); // Attach a function to trigger when something is received.
}

void morseLoop(unsigned long nMillis) {
  digitalWrite(LED_BUILTIN, (morseState == 0 ? LOW : HIGH));  // we may overwrite the same state in each loop, that's OK.
}




//////////////////// A behavior to cycle the dash lights

#define LED_PIN 11             // which pin are LEDS connected to?
#define NUM_LEDS 32
#define COLOR_ORDER RGB
#define LED_TYPE WS2812B       // i'm using WS2811s, FastLED supports lots of different types.

#define MAX_BRIGHTNESS 255     // Thats full on, watch the power!
#define MIN_BRIGHTNESS 5       // set to a minimum of 25%

struct CRGB leds[NUM_LEDS];

void ledSetup() {
  delay(3000); // in case we do something stupid. We dont want to get locked out.  TODO: someone tell Ian what "locked out" refers to

  LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MAX_BRIGHTNESS);
}

void ledLoop(unsigned long nMillis) {
  // make the hue and brightness become functions of time.
  // 3 and 7 are prime numbers so the hue & brightness won't coincide
  int brightness = constrain((nMillis / 30) % 255, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  int hue        = constrain((nMillis / 70) % 255, MIN_BRIGHTNESS, MAX_BRIGHTNESS);

  // set all LEDs to a hue, with each one offset a bit -- should be a rotating rainbow effect
  for(int i = 0; i < NUM_LEDS; i++) {
    // leds[i] = CHSV(hue, 255, 255); // use this line if the below doesn't work
    leds[i] = CHSV((hue + (i * (255 / NUM_LEDS))) % 255, 255, 255);
  }

  // set brightness and apply
  FastLED.setBrightness(brightness); // not sure if there is a way (or a need) to do this per-LED
  FastLED.show();
}



////////////////// Arduino functions

void setup() {
  d3Setup();
  morseSetup();
  ledSetup();
}

void loop() {
  unsigned long currentMillis = millis();

  // note that these behaviors MUST NOT use delay() inside them -- instead, make sure
  // that they accept the current time in millis and make their behavior decisions off
  // of that number (typically by the mod operator: %)

  d3Loop(currentMillis);
  morseLoop(currentMillis);
  ledLoop(currentMillis);
}
