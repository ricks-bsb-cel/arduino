#include <Arduino.h>

#define LedRed D1
#define LedGreen D2
#define LedBlue D3

#define LedOn HIGH
#define LedOff LOW
class RgbLeds {
public:
  RgbLeds() {
    pinMode(LedRed, OUTPUT);
    digitalWrite(LedRed, LedOff);
    pinMode(LedGreen, OUTPUT);
    digitalWrite(LedGreen, LedOff);
    pinMode(LedBlue, OUTPUT);
    digitalWrite(LedBlue, LedOff);
  };
  void Off(int d = 0) {
    digitalWrite(LedGreen, LedOff);
    digitalWrite(LedBlue, LedOff);
    digitalWrite(LedRed, LedOff);
    if (d > 0) delay(d);
  };
  void Red(int d = 0) {
    digitalWrite(LedGreen, LedOff);
    digitalWrite(LedBlue, LedOff);
    digitalWrite(LedRed, LedOn);
    if (d > 0) delay(d);
  };
  void Green(int d = 0) {
    digitalWrite(LedBlue, LedOff);
    digitalWrite(LedRed, LedOff);
    digitalWrite(LedGreen, LedOn);
    if (d > 0) delay(d);
  };
  void Blue(int d = 0) {
    digitalWrite(LedRed, LedOff);
    digitalWrite(LedGreen, LedOff);
    digitalWrite(LedBlue, LedOn);
    if (d > 0) delay(d);
  };
};

// Coloque fora do loop o que deve ser inicializado apenas uma vez
RgbLeds led;

void setup() {
}

void loop() {
  led.Red(100);
  led.Green(100);
  led.Blue(100);
}
