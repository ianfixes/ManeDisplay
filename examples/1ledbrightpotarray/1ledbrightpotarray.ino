 #include <FastLED.h>
    #define NUM_LEDS 29
    #define DATA_PIN 3
 
#define LED_TYPE WS2812B 

#define MAX_BRIGHTNESS 155      // Thats full on, watch the power!
#define MIN_BRIGHTNESS 25       // set to a minimum of 25%

const int brightnessInPin = A2;

CRGB rawleds[NUM_LEDS];
CRGBSet leds(rawleds, NUM_LEDS);
CRGBSet leds1(leds(0,7));
CRGBSet leds2(leds(8,15));
CRGBSet leds3(leds(16,23));

struct CRGB * ledarray[] ={leds1, leds2, leds3}; 


     void setup() { 
       FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

       FastLED.setBrightness(MAX_BRIGHTNESS);
   }

void loop() {

  int mappedValue = map(analogRead(brightnessInPin), 0, 1023, 0, 255);
  FastLED.setBrightness(constrain(mappedValue, MIN_BRIGHTNESS, MAX_BRIGHTNESS));

  for (int i=0; i<7; i++)
{
   leds[i] = CRGB::Red;
}
for (int i=7; i<29; i++)
{
   leds[i] = CRGB::Blue;
}
FastLED.show();
}
   