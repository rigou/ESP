// SoundSensor.ino - Detect Hand clap with Aliexpress "Sound Sensor Module KY-038 LM393"
// 2025-12-07

// the sound board output is HIGH while the room is silent
// and drops to LOW when a sound is detected
#define SOUNDBOARD_GPIO 36

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup() {
  	Serial.begin(115200);
	while (!Serial) ;
	Serial.print('\n'); for (uint8_t idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } // debug

	pinMode(SOUNDBOARD_GPIO, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.printf("\nReady\n");
}

bool detect_sound(void) {
	const int SOUND_MAX_DURATION=100; // ms
	const int SILENCE_MIN_DURATION=100; // ms
	const int COOLDOWN_DURATION=500; // ms
	bool retval_bool=false;

	// detect states changes at the sound board output
	// and debounce these signals: 1 clap of the hands triggers 11 LOW drops at SOUNDBOARD_GPIO during 30 ms
	static bool Last_sensor_bool=HIGH; // the sound board output is HIGH while the room is silent 
	static unsigned long First_signal_lng=0;
	static unsigned long Last_signal_lng=0;
	static unsigned long Cooldown_End_lng=0; 
	unsigned long now=millis();

	if (Cooldown_End_lng && now < Cooldown_End_lng)
		return false;

	bool sensor_bool = digitalRead(SOUNDBOARD_GPIO);
	if (sensor_bool != Last_sensor_bool) {
		// the sound board output has changed
		Last_sensor_bool=sensor_bool;
		if (sensor_bool == LOW) {
			// a sound is detected
			if ((First_signal_lng == 0) || (now-First_signal_lng > SILENCE_MIN_DURATION)) {
				// init or timeout
				First_signal_lng=now;
				Cooldown_End_lng=0;
			}	
			Last_signal_lng=now;
			Serial.printf("%lu Sensor=%d First=%lu Last=%lu Dur=%lu\n", now, sensor_bool, First_signal_lng, Last_signal_lng, Last_signal_lng-First_signal_lng);
		}
	}
	if (sensor_bool == HIGH 
		&& Last_signal_lng != 0
		&& now-Last_signal_lng > SILENCE_MIN_DURATION 
		&& Last_signal_lng-First_signal_lng < SOUND_MAX_DURATION 
	) {
		First_signal_lng=0;
		Last_signal_lng=0;
		Cooldown_End_lng=now+COOLDOWN_DURATION;
		retval_bool=true;
	}
	return retval_bool;
}

// expect 2 sound detections
bool check_sound(void) {
	const int MIN_INTERVAL=500;
	const int MAX_INTERVAL=3000;
	bool retval_bool=false;

	unsigned long now=millis();
	static unsigned long Last_detection_time_lng=0;

	unsigned long interval_int=Last_detection_time_lng?now-Last_detection_time_lng:0;
	if (interval_int > MAX_INTERVAL)
		Last_detection_time_lng=0;
	
	if (detect_sound()) {
		//Serial.printf("%lu Last=%lu Inter=%lu\n", now, Last_detection_time_lng, interval_int);
		if (!Last_detection_time_lng)
			Last_detection_time_lng=now; // 1st detection
		else {
			if (interval_int > MIN_INTERVAL && interval_int < MAX_INTERVAL)
				retval_bool=true;
			Last_detection_time_lng=0;
		}
	}
	return retval_bool;
}

void loop() {
	if (check_sound()) {
		// toggle Led state
		static bool Led_State_bool=LOW;
		Led_State_bool=!Led_State_bool;
		digitalWrite(LED_BUILTIN, Led_State_bool);
		//Serial.println("Detected");
	}
}