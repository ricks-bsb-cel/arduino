/* Template Definition */
#define BLYNK_TEMPLATE_ID "TMPL2sbk86y1U"
#define BLYNK_TEMPLATE_NAME "DHT LCD Module"

#define BLYNK_FIRMWARE_VERSION "0.1.1"
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#define VirtualMessage V0
#define VirtualResetWiFi V99

/* DHT ------------------------------ */

#define VirtualTemperature V20
#define VirtualHumidity V21

#define DHTPIN D5

/* RS485 ------------------------------ */

// Comunicação Serial RS485
#include <SoftwareSerial.h>

// Defina os pinos para a comunicação RS485
#define RS485_RX_PIN D1  // GPIO5 (D1) 5
#define RS485_TX_PIN D2  // GPIO4 (D2) 4

#define SLAVE_ADDRESS 0
#define FUNCTION_CODE 1
#define BYTE 2
#define TEMP_HB 3
#define TEMP_LB 4
#define HUMIDITY_HB 5
#define HUMIDITY_LB 6
#define CRC_HB 7
#define CRC_LB 8

// Leitura dos registradores a partir do endereço 0x0001
static byte TempAndHumidity[] = {
  0x01,        // Endereço do dispositivo
  0x04,        // Código de função (ler Holding Registers)
  0x00, 0x01,  // Endereço inicial do registro (low byte e high byte)
  0x00, 0x02,  // Número de registros a serem lidos (low byte e high byte)
  0x20, 0x0B   // CRC (vamos calcular a seguir)
};

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
SoftwareSerial rs485Serial(RS485_RX_PIN, RS485_TX_PIN);

float HexToFloat(byte firstValue, byte secondValue) {
  char hexString[5];
  sprintf(hexString, "%02X%02X", firstValue, secondValue);
  unsigned long decimalValue = strtoul(hexString, NULL, 16);
  return static_cast<float>(decimalValue) / 10.0;
}

void ReadSensor485Sensor() {
  // Leitura sensor RS485
  int wait = 20;
  byte receivedPayload[8];
  int i = 0;
  bool success = false;

  DEBUG_PRINT("Request");
  // Limpa dados antigos
  while (rs485Serial.available()) {
    rs485Serial.read();
  }
  delay(100);

  rs485Serial.write(TempAndHumidity, 8);

  while (!rs485Serial.available() && wait > 0) {
    wait--;
    delay(100);
  }

  if (rs485Serial.available()) {
    Serial.println();
  }

  while (rs485Serial.available() && i < 8) {
    success = true;
    byte received = rs485Serial.read();
    receivedPayload[i] = received;
    i++;
  }

  if (success) {
    float tempResult = HexToFloat(receivedPayload[TEMP_HB], receivedPayload[TEMP_LB]);
    float humidityResult = HexToFloat(receivedPayload[HUMIDITY_HB], receivedPayload[HUMIDITY_LB]);

    DEBUG_PRINT(String(tempResult));
    DEBUG_PRINT(String(humidityResult));
  }
}



void BlinkTimer() {

  /* DHT */
  sensors_event_t t;
  sensors_event_t h;

  dht.temperature().getEvent(&t);
  dht.humidity().getEvent(&h);

  Blynk.beginGroup();
  if (!isnan(t.temperature)) {
    Blynk.virtualWrite(VirtualTemperature, t.temperature);
  }

  if (!isnan(h.relative_humidity)) {
    Blynk.virtualWrite(VirtualHumidity, h.relative_humidity);
  }
  Blynk.endGroup();

  ReadSensor485Sensor();
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

  BlynkEdgent.begin();
  timer.setInterval(10000L, BlinkTimer);  // 60 Seconds

  rs485Serial.begin(9600);  // Defina a taxa de transmissão conforme necessário
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
