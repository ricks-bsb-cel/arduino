/* Template Definition */
#define BLYNK_TEMPLATE_ID "TMPL2dhm-xT37"
#define BLYNK_TEMPLATE_NAME "4 Relay Module"

/*
brisa-2154254: syvxfcxh
claro_2G157197: 19157197
*/

/* Don't forguet to update firmware versions! */
#define BLYNK_FIRMWARE_VERSION "0.1.71"

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

/*
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
*/

#include "BlynkEdgent.h"
#include "Utils.h"

/* Virtual Pins Definition */

// #define DHTPIN D2  // What digital pin we're connected to

#define _PinMacAddress V5
#define _PinDeviceID V6

#define _TopMessage V0
#define _Version V99

#define _PinD1 V1
#define _PinD2 V2
#define _PinD3 V3

#define ConfRele1 V11
#define ConfRele2 V12
#define ConfRele3 V13

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

// DHT_Unified dht(DHTPIN, DHT11);
BlynkTimer timer;
Utils utils;

/* General Methods */
bool CallFirstUpdate = true;

/*
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
*/

void SetDigitalPin(int pin, bool active) {
  if (active) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
};

void RegistrateDevice() {
  String macAddress = WiFi.macAddress();
  String DeviceId = utils.MacToString(macAddress);

  Blynk.virtualWrite(_PinMacAddress, macAddress);
  Blynk.virtualWrite(_PinDeviceID, DeviceId);
  Blynk.virtualWrite(_Version, String(BLYNK_FIRMWARE_VERSION));

  Blynk.virtualWrite(_PinD1, !digitalRead(Rele1));
  Blynk.virtualWrite(_PinD2, !digitalRead(Rele2));
  Blynk.virtualWrite(_PinD3, !digitalRead(Rele3));

  Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
  Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
  Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));

  Lcd.Log("Device registered");
}

void TurnAllRelaysOff() {
  SetDigitalPin(Rele1, false);
  SetDigitalPin(Rele2, false);
  SetDigitalPin(Rele3, false);
}

/* Blynk events */

int RefreshEach = 6;  // 60 Seconds
int RefreshCount = 0;

void BlinkTimer() {  // Each 10 Seconds
  if (Blynk.connected()) {
    if (CallFirstUpdate) {
      CallFirstUpdate = false;
      Lcd.SetTopMessage("HeySensa " + String(BLYNK_FIRMWARE_VERSION));
      RegistrateDevice();
    } else {
      if (RefreshCount == 0) {
        RefreshCount = RefreshEach;
        // ReadDH11();
        Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
        Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
        Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));
      }

      RefreshCount--;
      Lcd.ShowOnSameLine(String(RefreshCount));
    }
  }
}

BLYNK_WRITE(_TopMessage) {  // Mudança da Msg do Topo
  String topMessage = param.asString();
  Lcd.SetTopMessage(topMessage);
};
BLYNK_WRITE(_PinD1) {
  bool active = param.asInt() == 1;
  SetDigitalPin(Rele1, active);
  Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
};
BLYNK_WRITE(_PinD2) {
  bool active = param.asInt() == 1;
  SetDigitalPin(Rele2, active);
  Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
};
BLYNK_WRITE(_PinD3) {
  bool active = param.asInt() == 1;
  SetDigitalPin(Rele3, active);
  Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));
};

void setup() {
  Serial.begin(74880);
  delay(100);

  Lcd.Log("Starting...");

  // Set Digital Pins
  pinMode(Rele1, OUTPUT);
  pinMode(Rele2, OUTPUT);
  pinMode(Rele3, OUTPUT);

  TurnAllRelaysOff();

  // dht.begin();

  BlynkEdgent.begin();
  Lcd.Log("OTA Started...");
  timer.setInterval(10000L, BlinkTimer);  // 10 Seconds
  Lcd.Log("Device ready...");

  // Try First update
  BlinkTimer();
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}
