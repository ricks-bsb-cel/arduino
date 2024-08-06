/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL2X9ejTS6V"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "NOczROi0KHXkLDeP-j4RK4bCn98So93Z"

#define BLYNK_FIRMWARE_VERSION "0.1.1"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <ArduinoJson.h>  // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <ArduinoJson.hpp>

#include <vector>
#include <iostream>
#include <sstream>
#include <string>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>

#include <BlynkSimpleEsp8266.h>

#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <eeboom.h>

#include "Util.h"
#include "LcdLog.h"
#include "EepromStorage.h"
#include "Portal.h"

// DHT11 Sensor
#include <DHT.h>

// Blync Pins - Defina todos aqui! Nada sair usando diretamente o id
#define _PinMacAddress V5
#define _PinDeviceID V6

#define _TopMessage V0
#define _ResetWiFi V99

#define _PinD1 V1
#define _PinD2 V2
#define _PinD3 V3
#define _PinD4 V4

#define Rele1 D0
#define Rele2 D1
#define Rele3 D2
#define Rele4 D7

// Na D8 não inicializa qdo na energia

#define ConfRele1 V11
#define ConfRele2 V12
#define ConfRele3 V13
#define ConfRele4 V14

#define _vPinTemperature V20
#define _vPinHumidity V21

// DHT11 Sensor
DHT dht_sensor(D4, DHT11);

// Your WiFi credentials.
// Set password to "" for open networks.
EepromStorage eeprom;
BlynkTimer timer;
LcdLog Lcd("HeySensa");
Portal SsidPortal(&Lcd, &eeprom, "HeySensa");

String currentWiFiSsid;
String currentWiFiPassword;

void SetDigitalPin(int pin, bool active) {
  if (active) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
};

void ReadLocalTempUmidity() {
  float temp = dht_sensor.readTemperature();
  Serial.println("Reading Humidity...");
  float humi = dht_sensor.readHumidity();

  if (!isnan(temp)) {
    Blynk.virtualWrite(_vPinTemperature, temp);
  }

  if (!isnan(humi)) {
    Blynk.virtualWrite(_vPinHumidity, humi);
  }
}

void SendStatus() {
  Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
  Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
  Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));
  Blynk.virtualWrite(ConfRele4, !digitalRead(Rele4));

  ReadLocalTempUmidity();
}

void SendInitStatus() {
  Blynk.virtualWrite(V1, !digitalRead(Rele1));
  Blynk.virtualWrite(V2, !digitalRead(Rele2));
  Blynk.virtualWrite(V3, !digitalRead(Rele3));
  Blynk.virtualWrite(V4, !digitalRead(Rele4));

  Blynk.virtualWrite(ConfRele1, !digitalRead(Rele1));
  Blynk.virtualWrite(ConfRele2, !digitalRead(Rele2));
  Blynk.virtualWrite(ConfRele3, !digitalRead(Rele3));
  Blynk.virtualWrite(ConfRele4, !digitalRead(Rele4));
}

// Mudança da Msg do Topo
BLYNK_WRITE(_TopMessage) {
  String topMessage = param.asString();
  Lcd.SetTopMessage(topMessage);
};

// WiFi Reset
BLYNK_WRITE(_ResetWiFi) {
  if (param.asInt() == 1) {
    Util util;

    Serial.println("WiFi Reseting...");
    delay(2000);

    eeprom.Reset();
    util.Reboot();
  }
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

BLYNK_CONNECTED() {
  Lcd.Log("Server Connected");
}

bool ConnectToWiFi(String ssid, String password) {
  Lcd.Log("Connecting to WiFi:");
  Lcd.Log(ssid);

  WiFi.begin(ssid, password);  // Async!
  int maxTries = 15;           // In seconds

  while (WiFi.status() != WL_CONNECTED && maxTries > 0) {
    maxTries--;
    delay(1000);
  }

  return (WiFi.status() == WL_CONNECTED);
}

void updateDataBerofeLogin() {
  Util util;
  String macAddress = WiFi.macAddress();
  String DeviceId = util.MacToString(macAddress);

  Blynk.virtualWrite(_PinMacAddress, macAddress);
  Blynk.virtualWrite(_PinDeviceID, DeviceId);

  Lcd.Log("Device registered");
}

bool BlynkConnect() {
  int BlynkRetry = 10;
  delay(1000);

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  while (!Blynk.connected() and BlynkRetry > 0) {
    Lcd.Log("Connecting to Server");
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect(10000);  // Ten Seconds...
    delay(1000);
    BlynkRetry--;
  }

  if (Blynk.connected()) {
    updateDataBerofeLogin();  // Atualiza dados iniciais importantes no Server
    SendInitStatus();
  } else {
    Lcd.Log("Can't connect to Server");
    Lcd.Log("Restarting...");
    delay(2000);
    Util util;
    util.Reboot();
  }

  return Blynk.connected();
}

void RebootDevice() {
  Lcd.Log("Rebooting");
  delay(2000);
  Util util;
  util.Reboot();
}

void TurnAllOff() {
  SetDigitalPin(Rele1, false);
  SetDigitalPin(Rele2, false);
  SetDigitalPin(Rele3, false);
  SetDigitalPin(Rele4, false);
}

void setup() {
  Serial.begin(74880);

  // DHT11 Sensor
  dht_sensor.begin();

  Lcd.Log("Starting...");

  // Pinos Digitais
  pinMode(Rele1, OUTPUT);
  pinMode(Rele2, OUTPUT);
  pinMode(Rele3, OUTPUT);
  pinMode(Rele4, OUTPUT);

  TurnAllOff();

  if (eeprom.isLoaded()) {
    WiFiConfig eeData = eeprom.Data();

    currentWiFiSsid = eeData.ssid;
    currentWiFiPassword = eeData.password;

    if (ConnectToWiFi(eeData.ssid, eeData.password)) {
      Lcd.Log("WiFi Connected");

      if (BlynkConnect()) {
        Lcd.Log("I'm Ready!");
      }
    } else {
      Lcd.Log("Connection Error");
      SsidPortal.Start();
    }

    timer.setInterval(60000L, SendStatus); // 1 Minuto
  } else {
    Blynk.disconnect();
    Lcd.Log("WiFi Setup Not Found");
    SsidPortal.Start();
  }
}

void loop() {

  // Lida com desconexão do WiFi
  if (WiFi.status() != WL_CONNECTED && !SsidPortal.Active()) {
    Lcd.Log("WiFi not connected");
    int Retries = 10;

    while (WiFi.status() != WL_CONNECTED && Retries > 0) {
      Lcd.Log("-----------------------");
      Lcd.Log("Retrying " + String(Retries));
      if (ConnectToWiFi(currentWiFiSsid, currentWiFiPassword)) {
        BlynkConnect();
      }
      Retries--;
    }

    // Se depois ainda não tiver conseguido...
    if (WiFi.status() != WL_CONNECTED) {
      Lcd.Log("Can't reconnect...");
      TurnAllOff();
      RebootDevice();
    }
  }

  /* Blynk desconectada com WiFi conectada */
  if (WiFi.status() == WL_CONNECTED && !SsidPortal.Active()) {
    if (!Blynk.connected()) {
      BlynkConnect();
    }
  }

  if (Blynk.connected()) {
    Blynk.run();
    timer.run();
  }

  SsidPortal.Loop();  // Só executa se ativo...
}
