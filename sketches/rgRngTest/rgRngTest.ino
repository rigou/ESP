#include <rgRng.h>

rgRng Random_obj;

void setup() {
	Serial.begin(115200);
	while (!Serial);
	for (int idx=0; idx<=5; ++idx) {
		Serial.print(".");
		delay(100);
	}
    Serial.print('\n');
    Serial.print(RNGLIB_NAME);
	Serial.print(" ");
	Serial.println(RNGLIB_VERSION);
    Serial.print('\n');

    for (int idx=1; idx<=3; idx++) {
        Serial.printf("iteration %d\n", idx);
        Random_obj.Seed(100555);
        for (int idx=0; idx<=5; ++idx)
            Serial.printf("%u ", Random_obj.Next());
        Serial.print('\n');
        for (int idx=0; idx<=10; ++idx)
            Serial.printf("%2u ", Random_obj.Next(100));
        Serial.print('\n');
    }
}

void loop() {
    /* while (true) {
        for (int idx=0; idx<=10; ++idx)
            Serial.printf("%02u ", Random_obj.Next(100));
        Serial.println(' ');
        delay(1000);
    } */
}
