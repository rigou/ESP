#include "rgSerialBT.h"

#define APP_NAME	"rgSerialBTtest"
#define APP_VERSION	"v1.0.0"

const char *BTDEVID="esp01";
const int BTBUFFSIZE=100;
extern bool bt_Connected;

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	// give user some time to open the serial monitor
	int count_int=0;
	Serial.println("");
	while (count_int<5) {
		Serial.printf("%c", 'A'+count_int++);
		delay(1000);
	}
	Serial.printf("\n%s %s\n", APP_NAME, APP_VERSION);
	Serial.printf("\nBT device %s\n", BTDEVID);
	if (!bt_Begin(BTDEVID))
		Serial.println("Bluetooth failed");
}

void loop() {
	char text_buffer[BTBUFFSIZE];
	static unsigned long Last_time=0;
	static int Count=0;
	static bool Last_state=false; // true=client is connected

	if (bt_Connected!=Last_state) {
		Last_state=bt_Connected;
		Serial.println(bt_Connected?"Connected":"Disconnected");
	}

	if (millis() > Last_time+1000) {
		Last_time=millis();
		Count++;
		Serial.printf("%s : %d (%c)\n", BTDEVID, Count, bt_Connected?'+':'-');
		bt_Printf("%s : %d (%c)\n", BTDEVID, Count, bt_Connected?'+':'-');
	}
		
	int readlen=bt_Read(text_buffer, BTBUFFSIZE);
	if (readlen) {
		char text_str[BTBUFFSIZE];
		snprintf(text_str, BTBUFFSIZE-1, "Client : %.30s", text_buffer);
		Serial.println(text_str);
		bt_Writeln(text_str);
	}
}
