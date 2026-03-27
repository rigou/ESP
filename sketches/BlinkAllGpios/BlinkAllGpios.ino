/* Blink all GPIOS sequentially to locate the buit-in Led
 *  2024-11-18
 */

byte Pwm_gpios[]={0,  2,  4,  5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};

void setup() {
	
	Serial.begin(115200);
	while (!Serial) ;
	
	byte idx = 0;
	Serial.print('\n'); for (idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } ; Serial.print('\n');
	for (idx=0; idx<sizeof(Pwm_gpios); idx++)
		pinMode(Pwm_gpios[idx], OUTPUT);
}

void loop() {
	int idx=0;
	while (true) {
		Serial.printf("gpio %d\n", Pwm_gpios[idx]);
		digitalWrite(Pwm_gpios[idx], HIGH);
		delay(1000);
		digitalWrite(Pwm_gpios[idx], LOW);
		delay(1000);
		idx++;
		if (idx==sizeof(Pwm_gpios))
			idx=0;
	}
}
