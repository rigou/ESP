#include "arduinotx_led.h"
#include "arduinotx_lib.h"

// Led manager
ArduinotxLed Led_obj(LED_BUILTIN);

void setup() {
	Serial.begin(9600);
	stdout = stderr = fdevopen(serialWrite, NULL); // see aprintf() in arduinotx_lib.cpp
	Led_obj.SetCode('1');
}

void loop() {
	Led_obj.Flash();
}
