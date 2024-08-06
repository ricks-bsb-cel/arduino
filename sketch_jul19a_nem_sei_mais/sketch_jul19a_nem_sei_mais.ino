/**
 *  Modbus master example 1:
 *  The purpose of this example is to query an array of data
 *  from an external Modbus slave device. 
 *  The link media can be USB or RS232.
 *
 *  Recommended Modbus slave: 
 *  diagslave http://www.modbusdriver.com/diagslave.html
 *
 *  In a Linux box, run 
 *  "./diagslave /dev/ttyUSB0 -b 19200 -d 8 -s 1 -p none -m rtu -a 1"
 * 	This is:
 * 		serial port /dev/ttyUSB0 at 19200 baud 8N1
 *		RTU mode and address @1
 */

#include <ModbusRtu.h>

// data array for modbus network sharing
uint16_t au16data[16];
uint8_t u8state;
uint8_t slaveAddress = 0;

/**
 *  Modbus object declaration
 *  u8id : node id = 0 for master, = 1..247 for slave
 *  port : serial port
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */
Modbus master(0, Serial1, 1);  // this is master and RS-232 or USB-FTDI

/**
 * This is an structe which contains a query to an slave device
 */
modbus_t telegram;

unsigned long u32wait;

void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);  // baud-rate at 19200
  delay(1000);
  master.start();
  master.setTimeOut(5000);  // if there is no answer in 2000 ms, roll over
  u32wait = millis() + 1000;
  u8state = 0;
}

void loop() {
  switch (u8state) {
    case 0:
      if (millis() > u32wait) u8state++;  // wait state
      break;
    case 1:
      slaveAddress++;
      telegram.u8id = slaveAddress;  // slave address
      telegram.u8fct = 3;            // function code (this one is registers read)
      telegram.u16RegAdd = 1;        // start address in slave
      telegram.u16CoilsNo = 1;       // number of elements (coils or registers) to read
      telegram.au16reg = au16data;   // pointer to a memory array in the Arduino

      Serial.println();
      Serial.print("Query Send. Slave Address: ");
      Serial.println(slaveAddress);

      master.query(telegram);  // send query (only once)
      u8state++;
      break;
    case 2:
      master.poll();  // check incoming messages
  
      if (master.getState() == COM_IDLE) {
        u8state = 0;
        u32wait = millis() + 100;

        for (int i = 0; i < 16; i++) {
          Serial.print(au16data[i]);
        }
      } else {
        Serial.println(master.getState());
      }
      delay(2000);
      break;
  }
}
