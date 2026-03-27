/* SerialFileManager - Manages the root directory of a LittleFS file system in the flash storage of the ESP32
 * this implementation does not manage subdirectories
*/
#include <Arduino.h> // quiet vscode
#include "UserCmd.h"

#define APP_NAME "SerialFileManager"
#define APP_VERSION "1.3.0"

static UserCmd UserCmd_obj;

static const size_t MAX_INPUT_LEN=63; // not including the trailing zero
		
void setup() {
	Serial.begin(UserCmd::CMD_SERIAL_SPEED, SERIAL_8N1);
	while (!Serial) ; // wait for serial port to connect   
	Serial.print('\n'); for (uint8_t idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } // debug
	Serial.printf("\n\n%s %s using %s %s\n", APP_NAME, APP_VERSION, CMDLIB_NAME, CMDLIB_VERSION);

	// initialize the command interpreter 
    UserCmd_obj.Setup();
	UserCmd_obj.PrintHelp();
}

void loop() {
	if (Serial.available()) {
		// process command received over the Serial connection
		UserCmd_obj.Input(MAX_INPUT_LEN);
	}
}
