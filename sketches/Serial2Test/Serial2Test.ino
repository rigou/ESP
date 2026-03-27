/* Serial2Test.ino - Example program showing how to use ESP32's UART1
  2024-12-21

  Receives from the main serial port, sends to SerialPort1
  Receives from SerialPort1, sends to the main serial port

  Setup:
  - attach Arduino IDE's Serial Monitor to the serial port of the ESP32 board /dev/ttyUSB0
  - connect a 3.3V USB TTL serial adapter to SerialPort1 /dev/ttyUSB1 (RX=GPIO 25, TX=GPIO 26)
  - attach minicom to SerialPort1 : minicom -b 9600 -D /dev/ttyUSB1

  Derived by R.G. from 
  https://www.arduino.cc/en/Tutorial/BuiltInExamples/MultiSerialMega
  https://microcontrollerslab.com/esp32-uart-communication-pins-example/
*/
#include <Arduino.h>
#include <HardwareSerial.h>

HardwareSerial SerialPort1(1); // use UART1

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect   
	Serial.print('\n'); for (uint8_t idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } ; Serial.print('\n'); // debug
	Serial.println("Serial connected");

	// The RX and TX pins of UART1 are assigned to GPIO10 and GPIO9 by default
	// but they are are connected to the flash memory in the ESP32 board.
	// Hence, we will have to reassign the pins for UART1 for serial communication. 
	// The ESP32 board is capable to use almost all GPIO pins for serial connections,
	// here we have reassigned GPIO25 as RX pin and GPIO26 as TX pin.
	SerialPort1.begin(9600, SERIAL_8N1, 25, 26); // RX=GPIO 25, TX=GPIO 26
	while (!SerialPort1) ; // wait for serial port to connect
	SerialPort1.print('\n'); for (uint8_t idx = 0; idx<8; idx++) { SerialPort1.print((char)('A'+idx)); delay(500); } // debug
	Serial.println("SerialPort1 connected");
}

void loop() {
  // read from port 1, send to port 0:
  if (SerialPort1.available()) {
    int inByte = SerialPort1.read();
    Serial.write(inByte);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    SerialPort1.write(inByte);
  }
}
