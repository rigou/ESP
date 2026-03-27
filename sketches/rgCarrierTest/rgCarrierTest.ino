#include "rgCarrier.h"

#define BUZZER_GPIO 4
#define BUZZER_FREQ	440 // sound freq for cpu speed=240 MHz, lower cpu speed gives lower freq

void setup() {
	Serial.begin(115200);
	while (!Serial) ;
	// give user some time to open the serial monitor
	int count_int=0;
	Serial.println("");
	while (count_int<5) {
		Serial.printf("%c", 'A'+count_int++);
		delay(1000);
	}
	Serial.printf("\nBuzzer on gpio %d\n", BUZZER_GPIO);
	Serial.println("");Serial.println("");
	pinMode(LED_BUILTIN, OUTPUT);
	rgcarrier_init(BUZZER_GPIO, BUZZER_FREQ, 127, 1); // duty cycle alters the sound volume: 2=minimum, 127=maximum
}

void loop() {
	 static unsigned int count_int=0;
	 Serial.printf("%d,",++count_int);
	 if (count_int%10==0)
	 	Serial.println("");
	digitalWrite(LED_BUILTIN, LOW);
	rgcarrier_start();
	delay(250); // 5 ms sound duration sounds like a tick, 100 ms like a beep
	rgcarrier_stop();
	digitalWrite(LED_BUILTIN, HIGH);
	delay(750);
}
