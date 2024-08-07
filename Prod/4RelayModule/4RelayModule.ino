/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
#define BLYNK_TEMPLATE_ID "TMPL2X9ejTS6V"
#define BLYNK_TEMPLATE_NAME "Demonstração Principal"
// #define BLYNK_AUTH_TOKEN "COojb25_fI1_SueTDL0R_SHdC8LAk86K"

#define BLYNK_FIRMWARE_VERSION "0.1.8"
#define BLYNK_PRINT Serial

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
#define USE_NODE_MCU_BOARD

#include <vector>
#include <iostream>
#include <sstream>
#include <string>

#include <DHT.h>

#include "BlynkEdgent.h"
#include "Utils.h"
// #include "LcdLog.h"
#include "RgbLed.h"

BlynkTimer timer;
Utils utils;
RgbLed led;

// DHT11 Sensor
// DHT dht_sensor(D2, DHT11);

/*
void ReadLocalTempUmidity() {
  float temp = dht_sensor.readTemperature();
  Serial.println("Reading Humidity...");
  float humi = dht_sensor.readHumidity();

  if (!isnan(temp)) {
    Serial.println(temp);
  }

  if (!isnan(humi)) {
    Serial.println(humi);
  }
}
*/

// Comunicação Serial RS485
#include <SoftwareSerial.h>

// Defina os pinos para a comunicação RS485
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

void ReadSensor485Sensor() {
  // Leitura sensor RS485
  int wait = 20;
  byte receivedPayload[8];
  int i = 0;
  bool success = false;

  Serial.println("Request");
  // Limpa dados antigos
  while (rs485Serial.available()) {
    rs485Serial.read();
  }
  delay(100);

  rs485Serial.write(TempAndHumidity, 8);

  while (!rs485Serial.available() && wait > 0) {
    // Serial.print(".");
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
    Serial.print(received, HEX);  // Exibir os dados recebidos em decimal
    Serial.print(" ");
    i++;
  }

  if (success) {
    Serial.println();
    Serial.print("Temp: ");
    Serial.print(receivedPayload[TEMP_HB]);
    Serial.print(receivedPayload[TEMP_LB]);
    Serial.print(" ");
    float tempResult = utils.HexToFloat(receivedPayload[TEMP_HB], receivedPayload[TEMP_LB]);
    Serial.print(tempResult);

    Serial.println();
    Serial.print("Humidity: ");
    float humidityResult = utils.HexToFloat(receivedPayload[HUMIDITY_HB], receivedPayload[HUMIDITY_LB]);
    Serial.print(humidityResult);
    Serial.println();
  }
}

void BlinkTimer() {
  Serial.println("BlinkTimer " + String(BLYNK_FIRMWARE_VERSION));
  ReadSensor485Sensor();
}

void setup() {
  Serial.begin(74880);
  delay(100);

  rs485Serial.begin(9600);  // Defina a taxa de transmissão conforme necessário

  BlynkEdgent.begin();

  timer.setInterval(10000L, BlinkTimer);  // 1 Minuto
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}
