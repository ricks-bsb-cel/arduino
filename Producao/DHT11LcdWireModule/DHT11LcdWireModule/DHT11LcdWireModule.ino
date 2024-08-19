/* Template Definition */
#define BLYNK_TEMPLATE_ID "TMPL22uE5Viay"
#define BLYNK_TEMPLATE_NAME "DHT11WireModule"

#define BLYNK_FIRMWARE_VERSION "0.1.52"
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#define VirtualMessage V0
#define VirtualResetWiFi V99

/* DHT ------------------------------ */
#define VirtualTemperature V1
#define VirtualHumidity V2
#define VirtualVoltage V3

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
#include <String.h>

#include "BlynkEdgent.h"

BlynkTimer timer;
DHT_Unified dht(DHTPIN, DHT11);

#define _LcdPageLogData 0
#define _LcdPageTemperature 1
#define _LcdPageHumidity 2
#define _LCDPageVoltage 3

int LcdCurrentPage = 0;

void BlinkTimer() {

  /* DHT */
  sensors_event_t t;
  sensors_event_t h;

  dht.temperature().getEvent(&t);
  dht.humidity().getEvent(&h);

  /* Voltagem no pino A0 */
  float volt = 0.0;
  unsigned int raw = 0;

  raw = analogRead(A0);
  volt = raw / 1023.0;
  volt = volt * 5.18;

  if (LcdCurrentPage == 0) {
    Blynk.beginGroup();
    if (!isnan(t.temperature)) {
      Blynk.virtualWrite(VirtualTemperature, t.temperature);
    }

    if (!isnan(h.relative_humidity)) {
      Blynk.virtualWrite(VirtualHumidity, h.relative_humidity);
    }

    Blynk.virtualWrite(VirtualVoltage, volt);
    Blynk.endGroup();
  }

  if (!Blynk.connected()) {
    LcdCurrentPage = _LcdPageLogData;
  }

  if (LcdCurrentPage == _LcdPageLogData) {
    Lcd.ShowLogData();
  }

  if (LcdCurrentPage == _LcdPageTemperature && !isnan(t.temperature)) {
    Lcd.ShowBigValue("Temp C", String(t.temperature, 1), 10);
  }

  if (LcdCurrentPage == _LcdPageHumidity && !isnan(t.relative_humidity)) {
    Lcd.ShowBigValue("Humidity %", String(h.relative_humidity, 0), 10);
  }

  if (LcdCurrentPage == _LCDPageVoltage) {
    Lcd.ShowBigValue("Voltage", String(volt, 2), 10);
  }

  LcdCurrentPage++;

  if (LcdCurrentPage > 3) {
    LcdCurrentPage = _LcdPageLogData;
  }
}

/* Blink Remote Activation ------------------------------ */
BLYNK_WRITE(VirtualMessage) {
  String msg = param.asString();
  if (!msg.isEmpty()) {
    Lcd.Log(msg);
  }
};

BLYNK_WRITE(VirtualResetWiFi) {
  if (param.asInt() == 1) {
    Lcd.Log("Resetting config");
    delay(1000);
    enterResetConfig();
  }
};

void Initiate() {
  if (Blynk.connected()) {
    Lcd.Log("Initiated");
  }
}
/* ------------------------------------------------------ */

void setup() {
  ShowOnLcd("Starting");
  Serial.begin(74880);
  delay(100);

  pinMode(A0, INPUT);

  BlynkEdgent.begin();
  timer.setInterval(15000L, BlinkTimer);  // 60 Seconds

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
