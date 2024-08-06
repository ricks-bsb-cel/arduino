#include <SoftwareSerial.h>

// Defina os pinos para a comunicação RS485
#define RS485_RX_PIN 12  // GPIO12 (D6)
#define RS485_TX_PIN 13  // GPIO13 (D7)

#define SOFTWARE_SERIAL_BAUD 9600

#define SLAVE_ADDRESS 0
#define FUNCTION_CODE 1
#define BYTE 2
#define TEMP_HB 3
#define TEMP_LB 4
#define HUMIDITY_HB 5
#define HUMIDITY_LB 6
#define CRC_HB 7
#define CRC_LB 8

// No Setup
// rs485Serial.begin(SOFTWARE_SERIAL_BAUD);

// Vamos testar -------------------------
static byte TempAndHumidity[] = {
  0x01,        // Endereço do dispositivo
  0x04,        // Código de função (ler Holding Registers)
  0x00, 0x01,  // Endereço inicial do registro (low byte e high byte)
  0x00, 0x02,  // Número de registros a serem lidos (low byte e high byte)
  0x20, 0x0B   // CRC (vamos calcular a seguir)
};

