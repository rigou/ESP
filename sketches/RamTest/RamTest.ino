// source: https://thingpulse.com/esp32-how-to-use-psram/
// 2023-07-25

#include <Arduino.h>

void setup() {
	const int SERIAL_BAUDRATE = 115200;
	
	Serial.begin(SERIAL_BAUDRATE);
	while (!Serial) ; // wait for serial port to connect
	// give user some time to open the serial monitor
	Serial.println("");
	int idx = 0;
	while (idx < 12) {
		Serial.print((char)('A'+idx++));
		delay(500);
	}
	
	/*  ESP32-WROVER-E
		Select board "ESP32 Wrover Module" in Arduino IDE
		Sketch uses 277369 bytes (21%) of program storage space. Maximum is 1310720 bytes.
		Global variables use 22432 bytes (6%) of dynamic memory, leaving 305248 bytes for local variables. Maximum is 327680 bytes.

		Total heap: 369540
		Free heap: 344628
		Total PSRAM: 4192123
		Free PSRAM: 4192123
	*/
	Serial.println("");
	Serial.printf("Total heap: %d\n", ESP.getHeapSize());
	Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
	Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
	Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
}

void loop() {}
