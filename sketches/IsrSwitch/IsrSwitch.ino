#define SW1_PIN 15

bool volatile Sw1Pressed_bool=false;

void sw1_isr() {
	// debounce the switch event
	static unsigned long Last_event_lng=0;
	unsigned long this_event_lng=millis();
	if (this_event_lng-Last_event_lng > 500) {
		Sw1Pressed_bool=true;
		Last_event_lng=this_event_lng;
	}
}

void setup() {
  	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	delay(500);

	pinMode(SW1_PIN, INPUT_PULLUP);
	attachInterrupt(SW1_PIN, sw1_isr, FALLING);

	Serial.printf("Ready\n");
}

void loop() {
	if (Sw1Pressed_bool) {
		Sw1Pressed_bool = false;
		Serial.printf("Pressed\n");
	}
}
