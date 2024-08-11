/* Template Definition */
#define BLYNK_TEMPLATE_ID "TMPL2dhm-xT37"
#define BLYNK_TEMPLATE_NAME "4 Relay Module"

/* Don't forguet to update firmware versions! */
#define BLYNK_FIRMWARE_VERSION "0.1.4"

#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_NODE_MCU_BOARD


/* Main Libs */

#include <Arduino.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <string>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "BlynkEdgent.h"
#include "Utils.h"

/* Virtual Pins Definition */

#define DHTPIN D2  // What digital pin we're connected to

#define _PinMacAddress V5
#define _PinDeviceID V6

#define _TopMessage V0
#define _ResetWiFi V99

#define _PinD1 V1
#define _PinD2 V2
#define _PinD3 V3
// #define _PinD4 V4

#define ConfRele1 V11
#define ConfRele2 V12
#define ConfRele3 V13
// #define ConfRele4 V14

#define _vPinTemperature V20
#define _vPinHumidity V21

/* Real Pins Definition (do not use D8!) */
/* Never Use D8 */
#define Rele1 D0
#define Rele2 D1
#define Rele3 D7

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

/* Initiators */
/* Never Use D8. D4 is for Indicator Led */

DHT_Unified dht(DHTPIN, DHT11);
BlynkTimer timer;
Utils utils;

/* General Methods */
bool CallFirstUpdate = true;

void ReadDH11() {
  sensors_event_t t;
  sensors_event_t h;

  dht.temperature().getEvent(&t);
  dht.humidity().getEvent(&h);

  if (!isnan(t.temperature)) {
    Blynk.virtualWrite(_vPinTemperature, t.temperature);
  }

  if (!isnan(h.relative_humidity)) {
    Blynk.virtualWrite(_vPinHumidity, h.relative_humidity);
  }
}

void SendStatus() {
  Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
  Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
  Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));
}

void SetDigitalPin(int pin, bool active) {
  if (active) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
};

void StartDevice() {
  String macAddress = WiFi.macAddress();
  String DeviceId = utils.MacToString(macAddress);

  Blynk.beginGroup();

  Blynk.virtualWrite(_PinMacAddress, macAddress);
  Blynk.virtualWrite(_PinDeviceID, DeviceId);

  Blynk.virtualWrite(_PinD1, !digitalRead(Rele1));
  Blynk.virtualWrite(_PinD2, !digitalRead(Rele2));
  Blynk.virtualWrite(_PinD3, !digitalRead(Rele3));

  Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
  Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
  Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));

  Blynk.endGroup();

  Lcd.Log("Device registered");
}

void TurnAllRelaysOff() {
  SetDigitalPin(Rele1, false);
  SetDigitalPin(Rele2, false);
  SetDigitalPin(Rele3, false);
}

/* Blynk events */

void BlinkTimer() {
  if (Blynk.connected() && CallFirstUpdate) {
    CallFirstUpdate = false;
    Lcd.SetTopMessage("HeySensa " + String(BLYNK_FIRMWARE_VERSION));
    StartDevice();
  }

  Blynk.beginGroup();
  SendStatus();
  ReadDH11();
  Blynk.endGroup();
}

BLYNK_WRITE(_TopMessage) {  // Mudança da Msg do Topo
  String topMessage = param.asString();
  Lcd.SetTopMessage(topMessage);
};
BLYNK_WRITE(_PinD1) {
  bool active = param.asInt() == 1;
  SetDigitalPin(Rele1, active);
  SendStatus();
};
BLYNK_WRITE(_PinD2) {
  bool active = param.asInt() == 1;
  SetDigitalPin(Rele2, active);
  SendStatus();
};
BLYNK_WRITE(_PinD3) {
  bool active = param.asInt() == 1;
  SetDigitalPin(Rele3, active);
  SendStatus();
};

void setup() {
  Serial.begin(74880);
  delay(100);

  Lcd.Log("Starting...");

  dht.begin();

  // Set Digital Pins
  pinMode(Rele1, OUTPUT);
  pinMode(Rele2, OUTPUT);
  pinMode(Rele3, OUTPUT);

  TurnAllRelaysOff();

  BlynkEdgent.begin();
  timer.setInterval(30000L, BlinkTimer);  // 30 Seconds

  BlinkTimer();
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}
