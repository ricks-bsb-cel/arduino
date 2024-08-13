/* Template Definition */
#define BLYNK_TEMPLATE_ID "TMPL2dhm-xT37"
#define BLYNK_TEMPLATE_NAME "4 Relay Module"

#define BLYNK_FIRMWARE_VERSION "0.1.33"
#define BLYNK_PRINT Serial
// #define BLYNK_DEBUG
// #define APP_DEBUG
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

/* DHT ------------------------------ */

#define VirtualTemperature V20
#define VirtualHumidity V21

#define DHTPIN D2

/* Lcd ------------------------------ */

#include "LcdLog.h"
LcdLog Lcd("HeySensa " + String(BLYNK_FIRMWARE_VERSION), true);
#define ShowOnLcd(msg) \
  { Lcd.Log(msg); }
#define ShowOnLcdSameLine(msg) \
  { Lcd.ShowOnSameLine(msg); }

/* ------------------------------------------- */

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "BlynkEdgent.h"

BlynkTimer timer;
DHT_Unified dht(DHTPIN, DHT11);

void BlinkTimer() {

  /* DHT */
  sensors_event_t t;
  sensors_event_t h;

  dht.temperature().getEvent(&t);
  dht.humidity().getEvent(&h);

  Blynk.beginGroup();
  Blynk.virtualWrite(VirtualConfRele1, !digitalRead(PinRele1));
  Blynk.virtualWrite(VirtualConfRele2, !digitalRead(PinRele2));
  Blynk.virtualWrite(VirtualConfRele3, !digitalRead(PinRele3));

  if (!isnan(t.temperature)) {
    Blynk.virtualWrite(VirtualTemperature, t.temperature);
  }

  if (!isnan(h.relative_humidity)) {
    Blynk.virtualWrite(VirtualHumidity, h.relative_humidity);
  }

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

void Initiate() {
  // Ensure client is updated
  if (Blynk.connected()) {
    Blynk.beginGroup();
    Blynk.virtualWrite(VirtualRele1, 0);
    Blynk.virtualWrite(VirtualRele2, 0);
    Blynk.virtualWrite(VirtualRele3, 0);

    Blynk.virtualWrite(VirtualConfRele1, 0);
    Blynk.virtualWrite(VirtualConfRele2, 0);
    Blynk.virtualWrite(VirtualConfRele3, 0);
    Blynk.endGroup();

    Lcd.Log("Initiated");
  }
}

/* ------------------------------------------------------ */

void setup() {
  ShowOnLcd("Starting");
  Serial.begin(74880);
  delay(100);

  BlynkEdgent.begin();
  timer.setInterval(60000L, BlinkTimer);  // 60 Seconds

  // Set Digital Pins
  pinMode(PinRele1, OUTPUT);
  pinMode(PinRele2, OUTPUT);
  pinMode(PinRele3, OUTPUT);

  // Ensure pins are off
  SetDigitalPin(PinRele1, false);
  SetDigitalPin(PinRele2, false);
  SetDigitalPin(PinRele3, false);

  dht.begin();

  ShowOnLcd("Setup ready");
}

bool isInitiated = false;

void loop() {
  BlynkEdgent.run();
  timer.run();

  if (!isInitiated && Blynk.connected()) {
    isInitiated = true;
    Initiate();
  }
}
