#include <KMP_RS485.h>

KMP_RS485 _rs485(Serial1, 19 /*tePin*/, 18 /*TX pin*/); 
// This configuration is liked with PRODINo MKR Zero you can change it to fit with your needs

void setup()
{
	Serial.begin(19200);

	_rs485.begin(19200); // Start RS-485 communication

  _rs485.beginTransmission();
	_rs485.print(data);
	_rs485.endTransmission();
}

void loop() {
	// Exit if data is not available
	if (!_rs485.available()) {
		return;
	}
	
	String data = _rs485.readString(); // Read data as string

	// Print debug data
	Serial.print("Received data: ");
	Serial.println(data);

	// Transmit data back
	_rs485.beginTransmission();
	_rs485.print(data);
	_rs485.endTransmission();
}