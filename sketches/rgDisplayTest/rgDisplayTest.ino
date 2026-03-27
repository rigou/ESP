#include <rgDisplay.h>

#define DEBUG_ON 1
#include "rgDebug.h"

rgDisplay Display_obj;

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
#if DEBUG_ON
	dbprint('\n'); for (uint8_t idx = 0; idx<6; idx++) { dbprint((char)('A'+idx)); delay(500); }
	dbprint('\n'); 
#endif
	if (!Display_obj.Init()) {
		dbprintln("Display initialization failed");
		while(1);
	}
	Display_obj.set_FontSize(2); // font_size 2 : 4 x 10     char height=16 char width=12 -> 4 lines 10 characters/line
	Display_obj.PrintLine("abcdefghij", 2);
}

void loop() {
	const char *TXT[]={"A", "QUICK", "BLUE", "FOX"};
	while (1) {
		delay(500);
		for (byte idx=0; idx<=3; idx++) {
			unsigned long start_tim=micros();
			Display_obj.PrintLine(TXT[idx], 1);
			Serial.printf("%lu microseconds\n", micros()-start_tim);
			delay(500);
		}
	}
}
