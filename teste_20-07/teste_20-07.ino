/*
  Modbus Library for Arduino Example - Modbus RTU Client
  Read Holding Registers from Modbus RTU Server in blocking way
  ESP8266 Example
  
  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define SLAVE_ID 0x01
#define FIRST_REG 0
#define REG_COUNT 3

// #define RS485_RX_PIN 13 // GPIO13 (D7)
// #define RS485_TX_PIN 15 // GPIO15 (D8)

// #define RS485_RX_PIN 3 // GPIO3 (D9)
// #define RS485_TX_PIN 1 // GPIO1 (D10)

#define RS485_RX_PIN 5 // GPIO5 (D1)
#define RS485_TX_PIN 4 // GPIO4 (D2)

SoftwareSerial S(RS485_RX_PIN, RS485_TX_PIN);
ModbusRTU mb;

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) {  // Callback to monitor errors
  if (event != Modbus::EX_SUCCESS) {
    Serial.print("Request result: 0x");
    Serial.print(event, HEX);
  }
  return true;
}

void setup() {
  Serial.begin(9600);
  S.begin(9600, SWSERIAL_8N1);
  mb.begin(&S);
  mb.client();
}

void loop() {
  uint16_t res[REG_COUNT];

  if (!mb.slave()) {  // Check if no transaction in progress
    // mb.readHreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
    // mb.readIreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
    mb.readIreg(SLAVE_ID, 0X0000, res, REG_COUNT, cb);  // Send Read Hreg from Modbus Server
    while (mb.slave()) {                                // Check if transaction is active
      mb.task();
      delay(10);
    }
    Serial.println("------");
    for (int i = 0; i < REG_COUNT; i++) {
      Serial.println(res[i], BIN);
    }
  }
  delay(5000);
}