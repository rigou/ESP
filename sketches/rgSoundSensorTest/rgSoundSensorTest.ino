// rgSoundSensorTest.ino - Detect Hand claps with Aliexpress "Sound Sensor Module KY-038 LM393"
// 2025-12-07

// the sound board output is HIGH while the room is silent
// and drops to LOW when a sound is detected
#define SOUNDBOARD_GPIO 36

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#include <rgSoundSensor.h>
rgSoundSensor Soundboard_obj(SOUNDBOARD_GPIO);

void setup() {
  	// Serial.begin(115200);
	// while (!Serial) ;
	// Serial.print('\n'); for (uint8_t idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } // debug

	pinMode(SOUNDBOARD_GPIO, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
	if (Soundboard_obj.Check()) {
		// toggle Led state
		static bool Led_State_bool=LOW;
		Led_State_bool=!Led_State_bool;
		digitalWrite(LED_BUILTIN, Led_State_bool);
		//Serial.println("Detected");
	}
}