/* Template Definition */
#define BLYNK_TEMPLATE_ID "TMPL2CAsz8cQ6"
#define BLYNK_TEMPLATE_NAME "RS485"

#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#define VirtualMessage V0
#define VirtualResetWiFi V99
#define VirtualTemperature V1
#define VirtualHumidity V2

/* Pinagem com o ESP8266 12-E ou Wemos D1 Mini 
LED SCL => GPIO12 D6
LED SDA => GPIO13 D7

TTL RXD => GPIO5
TTL TXD => GPIO4
*/

/* Lcd ------------------------------ */

#include "LcdLog.h"
LcdLog Lcd("HeySensa " + String(BLYNK_FIRMWARE_VERSION), true);
#define ShowOnLcd(msg) \
  { Lcd.Log(msg); }
#define ShowOnLcdSameLine(msg) \
  { Lcd.ShowOnSameLine(msg); }

/* ------------------------------------------- */
#include <SoftwareSerial.h>

#define RS485_RX_PIN 5  // GPIO5 (D1)
#define RS485_TX_PIN 4  // GPIO4 (D2)

#define SLAVE_ADDRESS 0
#define FUNCTION_CODE 1
#define BYTE 2
#define TEMP_HB 3
#define TEMP_LB 4
#define HUMIDITY_HB 5
#define HUMIDITY_LB 6
#define CRC_HB 7
#define CRC_LB 8

// Solicitar a leitura dos registradores a partir do endereço 0x0001
static byte TempAndHumidity[] = {
  0x01,        // Endereço do dispositivo
  0x04,        // Código de função (ler Holding Registers)
  0x00, 0x01,  // Endereço inicial do registro (low byte e high byte)
  0x00, 0x02,  // Número de registros a serem lidos (low byte e high byte)
  0x20, 0x0B   // CRC (vamos calcular a seguir)
};

SoftwareSerial rs485Serial(RS485_RX_PIN, RS485_TX_PIN);
/* ------------------------------------------- */

#include <String.h>

#include "BlynkEdgent.h"

BlynkTimer timer;

float HexToFloat(byte firstValue, byte secondValue) {
  char hexString[5];
  sprintf(hexString, "%02X%02X", firstValue, secondValue);
  unsigned long decimalValue = strtoul(hexString, NULL, 16);
  return static_cast<float>(decimalValue) / 10.0;
};

void BlinkTimer() {
  DEBUG_PRINT("BlinkTimer!");

  bool success = false;
  int wait = 20;
  int i = 0;
  byte receivedPayload[8];

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
    float temperature = HexToFloat(receivedPayload[TEMP_HB], receivedPayload[TEMP_LB]);
    float humidity = HexToFloat(receivedPayload[HUMIDITY_HB], receivedPayload[HUMIDITY_LB]);

    Blynk.virtualWrite(VirtualTemperature, temperature);
    Blynk.virtualWrite(VirtualHumidity, humidity);

    DEBUG_PRINT(String(temperature));
    DEBUG_PRINT(String(humidity));
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
  // Ensure client is updated
  if (Blynk.connected()) {
    Lcd.Log("Initiated");
  }
}

/* ------------------------------------------------------ */

void setup() {
  ShowOnLcd("Starting");
  Serial.begin(74880);
  rs485Serial.begin(9600);
  delay(100);

  BlynkEdgent.begin();
  timer.setInterval(10000L, BlinkTimer);  // 60 Seconds

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
