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

#include <vector>
#include <iostream>
#include <sstream>
#include <string>

#include "BlynkEdgent.h"
#include "Utils.h"

/* Virtual Pins Definition */

#define _PinMacAddress V5
#define _PinDeviceID V6

#define _TopMessage V0
#define _ResetWiFi V99

#define _PinD1 V1
#define _PinD2 V2
#define _PinD3 V3
#define _PinD4 V4

#define ConfRele1 V11
#define ConfRele2 V12
#define ConfRele3 V13
#define ConfRele4 V14

#define _vPinTemperature V20
#define _vPinHumidity V21

/* Real Pins Definition (do not use D8!) */
#define Rele1 D0
#define Rele2 D1
#define Rele3 D2
#define Rele4 D7

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

/* Initiators */

BlynkTimer timer;
Utils utils;

/* General Methods */
bool CallFirstUpdate = true;

void SendStatus() {
  Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
  Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
  Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));
  Blynk.virtualWrite(ConfRele4, !digitalRead(Rele4));

  // ReadLocalTempUmidity();
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
  Blynk.virtualWrite(_PinD4, !digitalRead(Rele4));

  Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
  Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
  Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));
  Blynk.virtualWrite(ConfRele4, !digitalRead(Rele4));

  Blynk.endGroup();

  Lcd.Log("Device registered");
}

void TurnAllRelaysOff() {
  SetDigitalPin(Rele1, false);
  SetDigitalPin(Rele2, false);
  SetDigitalPin(Rele3, false);
  SetDigitalPin(Rele4, false);
}

/* Blynk events */

void BlinkTimer() {
  // Serial.println("BlinkTimer " + String(BLYNK_FIRMWARE_VERSION));

  if (Blynk.connected() && CallFirstUpdate) {
    CallFirstUpdate = false;
    Lcd.SetTopMessage("HeySensa " + String(BLYNK_FIRMWARE_VERSION));
    StartDevice();
  }
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

BLYNK_WRITE(_PinD4) {
  bool active = param.asInt() == 1;
  SetDigitalPin(Rele4, active);
  SendStatus();
};

void setup() {
  Serial.begin(74880);
  delay(100);

  Lcd.Log("Starting...");

  BlynkEdgent.begin();

  timer.setInterval(10000L, BlinkTimer);  // 10 Seconds

  // Set Digital Pins
  pinMode(Rele1, OUTPUT);
  pinMode(Rele2, OUTPUT);
  pinMode(Rele3, OUTPUT);
  pinMode(Rele4, OUTPUT);

  TurnAllRelaysOff();
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}
