#include <rgButton.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const byte RUNLED=2 ; // LED_BUILTIN, defined in RF24_Lib.h
const byte BTN_GPIO=0;

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	// give user some time to open the serial monitor
	Serial.println("");
	int idx = 0;
	while (idx < 12) {
		Serial.print((char)('A'+idx++));
		delay(500);
	}
	Serial.println("\nsetup:");

	pinMode(RUNLED, OUTPUT);
	pinMode(BTN_GPIO, INPUT_PULLUP);

	BtnStates btn_state=ReadBtnBlock(BTN_GPIO, RUNLED, 3000);
	print_button_state(btn_state);

	Serial.println("loop:");
}

void loop() {
	BtnStates btn_state=ReadBtn(BTN_GPIO, RUNLED, 1500);
	print_button_state(btn_state);

	// if btn_state==BTN_PRESSED then ReadBtn() will manage the Led
	if (btn_state==BTN_RELEASED) { 
		digitalWrite(RUNLED, HIGH);
		delay(50);
		digitalWrite(RUNLED, LOW);
		delay(200);
	}
}

void print_button_state(BtnStates btn_state) {
	static byte Last_state=255;
	if (btn_state!=Last_state) {
		if (btn_state==BTN_REACHED_DURATION)
			Serial.println("BTN_REACHED_DURATION");
		else if (btn_state==BTN_RELEASED)
			Serial.println("BTN_RELEASED");
		else if (btn_state==BTN_PRESSED)
			Serial.println("BTN_PRESSED");
		Last_state=btn_state;
	}
}
