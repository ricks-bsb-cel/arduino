/* Template Definition */
#define BLYNK_TEMPLATE_ID "TMPL2dhm-xT37"
#define BLYNK_TEMPLATE_NAME "4 Relay Module"

#define BLYNK_FIRMWARE_VERSION "0.1.2"
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

/* Relay Config ------------------------------ */
#define PinRele1 D0
#define PinRele2 D1
#define PinRele3 D7

#define VirtualRele1 V1
#define VirtualRele2 V2
#define VirtualRele3 V3

#define VirtualConfRele1 V11
#define VirtualConfRele2 V12
#define VirtualConfRele3 V13
/* ------------------------------------------- */

#include "LcdLog.h"
LcdLog Lcd("HeySensa " + String(BLYNK_FIRMWARE_VERSION), true);
#define ShowOnLcd(msg) \
  { Lcd.Log(msg); }
#define ShowOnLcdSameLine(msg) \
  { Lcd.ShowOnSameLine(msg); }

#include "BlynkEdgent.h"

BlynkTimer timer;

void BlinkTimer() {
  DEBUG_PRINT("Timer!");

  Blynk.beginGroup();
  Blynk.virtualWrite(VirtualConfRele1, !digitalRead(PinRele1));
  Blynk.virtualWrite(VirtualConfRele2, !digitalRead(PinRele2));
  Blynk.virtualWrite(VirtualConfRele3, !digitalRead(PinRele3));
  Blynk.endGroup();
}

void SetDigitalPin(int pin, bool active) {
  if (active) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
};

/* Blink Remote Activation ------------------------------ */

BLYNK_WRITE(VirtualRele1) {
  SetDigitalPin(PinRele1, param.asInt() == 1);
  delay(100);
  Blynk.virtualWrite(VirtualConfRele1, !digitalRead(PinRele1));
};

BLYNK_WRITE(VirtualRele2) {
  SetDigitalPin(PinRele2, param.asInt() == 1);
  delay(100);
  Blynk.virtualWrite(VirtualConfRele2, !digitalRead(PinRele2));
};

BLYNK_WRITE(VirtualRele3) {
  SetDigitalPin(PinRele3, param.asInt() == 1);
  delay(100);
  Blynk.virtualWrite(VirtualConfRele3, !digitalRead(PinRele3));
};

/* ------------------------------------------------------ */

void setup() {
  ShowOnLcd("Starting");
  Serial.begin(74880);
  delay(100);

  BlynkEdgent.begin();
  timer.setInterval(30000L, BlinkTimer);  // 10 Seconds

  // Set Digital Pins
  pinMode(PinRele1, OUTPUT);
  pinMode(PinRele2, OUTPUT);
  pinMode(PinRele3, OUTPUT);

  // Ensure pins are off
  SetDigitalPin(PinRele1, false);
  SetDigitalPin(PinRele2, false);
  SetDigitalPin(PinRele3, false);

  ShowOnLcd("Setup ready");
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}
