/* SerialRxTest.ino - Receive a file over Serial, output log over bluetooth
** 2024-07-07
** Send a file: stty -F /dev/ttyUSB0 115200 && cat loremipsum.txt > /dev/ttyUSB0
*/

#include "rgSerialBT.h"
const char *BTDEVID="esp04";
const int BTBUFFSIZE=100;

void setup() {
	char bt_buffer[BTBUFFSIZE];
	if (!bt_Begin(BTDEVID))
		Serial.println("Bluetooth failed");
	for (uint8_t idx = 0; idx<10; idx++) {
		bt_buffer[0]=('A'+idx);
		bt_buffer[1]='\0';
		bt_Writeln(bt_buffer);
		delay(500); 
	}

	Serial.begin(115200, SERIAL_8N1);
	while (!Serial) ; // wait for serial port to connect
}

void loop() {
	const size_t RXBUFFSIZE=1001; // +1 for the terminating \0
	char rx_buffer[RXBUFFSIZE];
	char bt_buffer[BTBUFFSIZE];

	bt_Writeln("Ready to receive");
	size_t rx_length=receive_data(rx_buffer, RXBUFFSIZE);
	snprintf(bt_buffer, BTBUFFSIZE, "Received %u bytes", rx_length);
	bt_Writeln(bt_buffer);
	if (rx_length) {
		bt_Writeln("--------------------");
		bt_Writeln(rx_buffer);
		bt_Writeln("--------------------");
	}
}

// return value: number of characters received
size_t receive_data(char *buffer_out, size_t buffer_length) {
	unsigned long start_time=millis();
	unsigned long read_time=millis();
	const unsigned READ_TIMEOUT=100; // ms
	const unsigned START_TIMEOUT=30000; // ms
	size_t offset=0;

	// read serial input until nothing received for READ_TIMEOUT
	memset(buffer_out, 0, buffer_length);
	while (millis() <= read_time+READ_TIMEOUT || (offset==0 && millis() <= start_time+START_TIMEOUT)) {
		if (offset < buffer_length) {
			if (Serial.available()) {
				buffer_out[offset++]=Serial.read();
				read_time=millis();
			}
		}
		else
			break; // read buffer full
	}
	return offset;
}