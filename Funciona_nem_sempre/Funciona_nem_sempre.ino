#include <SoftwareSerial.h>
#include <Arduino.h>

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

// Crie uma instância de SoftwareSerial
SoftwareSerial rs485Serial(RS485_RX_PIN, RS485_TX_PIN);

// Solicitar a leitura dos registradores a partir do endereço 0x0001
static byte TempAndHumidity[] = {
  0x01,        // Endereço do dispositivo
  0x04,        // Código de função (ler Holding Registers)
  0x00, 0x01,  // Endereço inicial do registro (low byte e high byte)
  0x00, 0x02,  // Número de registros a serem lidos (low byte e high byte)
  0x20, 0x0B   // CRC (vamos calcular a seguir)
};

float HexToFloat(byte firstValue, byte secondValue) {
  char hexString[5];
  sprintf(hexString, "%02X%02X", firstValue, secondValue);
  unsigned long decimalValue = strtoul(hexString, NULL, 16);
  return static_cast<float>(decimalValue) / 10.0;
}

void setup() {
  // Inicialize a Serial hardware para comunicação com o computador
  Serial.begin(74880);
  Serial.println();
  Serial.println("Inicializando Serial...");

  // Inicialize a SoftwareSerial para RS485
  rs485Serial.begin(9600);  // Defina a taxa de transmissão conforme necessário
  Serial.println("RS485 inicializada.");

  /*
  // Calcular CRC
  Serial.println("Calculando CRC");
  unsigned int crc = calculateCRC(request, 6);
  request[6] = crc & 0xFF;         // CRC Low byte
  request[7] = (crc >> 8) & 0xFF;  // CRC High byte
  Serial.println("CRC Adicionado");
  */

  /*
  Serial.println("Enviando dados");
  delay(100); // Silêncio
  rs485Serial.write(request, 8);
  delay(100);
  */
}

void loop() {
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
    float tempResult = HexToFloat(receivedPayload[TEMP_HB], receivedPayload[TEMP_LB]);
    Serial.print(tempResult);

    Serial.println();
    Serial.print("Humidity: ");
    float humidityResult = HexToFloat(receivedPayload[HUMIDITY_HB], receivedPayload[HUMIDITY_LB]);
    Serial.print(humidityResult);
    Serial.println();

    delay(1000);  // Aguarda próxima tentativa de leitura
  }

  /*
  // Verifique se há dados disponíveis na RS485
  if (rs485Serial.available()) {
    Serial.println("rs485Serial available");

    while (rs485Serial.available()) {
      byte received = rs485Serial.read();
      Serial.print(received, HEX);  // Exibir os dados recebidos em decimal
      Serial.print(" ");
    }
    Serial.println();
  }else{
    Serial.print(".");
  }
  */

  delay(1000);  // Aguarda próxima tentativa de leitura
}

/*
// Função para calcular CRC
unsigned int calculateCRC(byte *buffer, byte length) {
  unsigned int crc = 0xFFFF;
  for (byte pos = 0; pos < length; pos++) {
    crc ^= (unsigned int)buffer[pos];
    for (byte i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}
*/
