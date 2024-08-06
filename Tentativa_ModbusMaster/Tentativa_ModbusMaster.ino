#include <ModbusMaster.h>
// http://4-20ma.io/ModbusMaster/group__buffer.html#gaef93f45298dfab2a7816bd38dac34bc9

ModbusMaster node;

#define MAX485_DE 3
#define MAX485_RE_NEG 2

#define TX_EN_PIN 18
#define RX_EN_PIN 19

/*
MAX485_RE_NEG: This variable likely represents the pin connected to the ~RE (Receive Enable) 
signal of the MAX485 transceiver. The _NEG suffix suggests that this is the active-low version of the signal.

MAX485_DE: This variable likely represents the pin connected to the DE (Driver Enable)
signal of the MAX485 transceiver.
*/

void preTransmission() {
  digitalWrite(MAX485_RE_NEG, HIGH);
  digitalWrite(MAX485_DE, HIGH);
  // digitalWrite(TX_EN_PIN, HIGH);
  // digitalWrite(RX_EN_PIN, LOW);
}

void postTransmission() {
  digitalWrite(MAX485_RE_NEG, LOW);
  digitalWrite(MAX485_DE, LOW);
  // digitalWrite(TX_EN_PIN, LOW);
  // digitalWrite(RX_EN_PIN, HIGH);
}

uint32_t i;
int tp_modbus;
int tp_status;

void setup() {
  i = 0;
  tp_modbus = 0;

  Serial.println("Setup Start");

  Serial.begin(19200);
  Serial1.begin(9600);

  // pinMode(TX_EN_PIN, OUTPUT);
  // pinMode(RX_EN_PIN, OUTPUT);

  // Init in receive mode
  // digitalWrite(MAX485_RE_NEG, 0);
  // digitalWrite(MAX485_DE, 0);

  // digitalWrite(TX_EN_PIN, LOW);
  // digitalWrite(RX_EN_PIN, HIGH);

  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  node.begin(1, Serial1);

  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("Setup Ready");
}


void loop() {
  i++;
  uint8_t result;

  node.begin(i, Serial1);

  node.setTransmitBuffer(0, lowWord(i));
  node.setTransmitBuffer(1, highWord(i));

  // read holding register - temperature
  const byte ph[] = {0x03, 0x0101}; // Soil PH Sensor

  result = node.readInputRegisters(ph, 2);

  if (result == node.ku8MBSuccess) {
    Serial.println("*****************************************************************************");
    tp_modbus = node.getResponseBuffer(0);
    Serial.println("*****************************************************************************");
  }

  tp_status = result;

  Serial.print(String(i) + ": ");

    if (tp_status == node.ku8MBIllegalFunction) {
    Serial.println("ku8MBIllegalFunction");
  }
  else if (tp_status == node.ku8MBIllegalDataAddress) {
    Serial.println("ku8MBIllegalDataAddress");
  }
  else if (tp_status == node.ku8MBIllegalDataValue) {
    Serial.println("ku8MBIllegalDataValue");
  }
  else if (tp_status == node.ku8MBSlaveDeviceFailure) {
    Serial.println("ku8MBSlaveDeviceFailure");
  }
  else if (tp_status == node.ku8MBInvalidSlaveID) {
    Serial.println("ku8MBInvalidSlaveID");
  }
  else if (tp_status == node.ku8MBInvalidFunction) {
    Serial.println("ku8MBInvalidFunction");
  }
  else if (tp_status == node.ku8MBResponseTimedOut) {
    Serial.println("ku8MBResponseTimedOut");
  }
  else if (tp_status == node.ku8MBInvalidCRC) {
    Serial.println("ku8MBInvalidCRC");
  }
  else {
    Serial.println(String(tp_modbus));
  }

  /*

  delay(500);
  uint8_t result;
  // result = node.readHoldingRegisters(0x04/0x0001, 2);
  Serial.println(result);
  Serial.println(node.getResponseBuffer(0));
  delay(500);


  uint8_t result;
  uint8_t s;

  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
  Serial.println("HIGH");

  result = node.readHoldingRegisters(1, 0x0101);
  Serial.println(result);
  Serial.println(node.getResponseBuffer(0));

  delay(10000);

  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.println("LOW");

  node.readHoldingRegisters(0x0101, 2);

  delay(2000);
  result = node.readHoldingRegisters(0x0101, 2);

  if (result == node.ku8MBSuccess) {
    sensorID = node.getResponseBuffer(0);
    Serial.print("Sensor ID: ");
    Serial.println(sensorID);
    // Armazene o ID do sensor em uma variável para uso posterior (se necessário)
  } else {
    Serial.println("Error reading sensor ID");
  }

  static uint32_t i;
  uint8_t j, result;
  uint16_t data[6];

  i++;


  Serial.println("set word 0 of TX buffer to least-significant word of counter (bits 15..0)");
  node.setTransmitBuffer(0, lowWord(i));

  Serial.println("set word 1 of TX buffer to most-significant word of counter (bits 31..16)");
  node.setTransmitBuffer(1, highWord(i));

  Serial.println("slave: write TX buffer to (2) 16-bit registers starting at register 0");
  result = node.writeMultipleRegisters(0, 2);

  Serial.println("slave: read (6) 16-bit registers starting at register 2 to RX buffer");
  result = node.readHoldingRegisters(2, 6);

  // do something with data if read is successful
  if (result == node.ku8MBSuccess) {
    for (j = 0; j < 6; j++) {
      data[j] = node.getResponseBuffer(j);
      Serial.println(String(data[j]));
    }
  }
  */
}