// rgSoundSensor - Detect Hand claps with Aliexpress "Sound Sensor Module KY-038 LM393"

#include "rgSoundSensor.h"

// Constructor
rgSoundSensor::rgSoundSensor(int gpio) 
	: Soundboard_gpio(gpio),
	Last_sensor_bool(HIGH),
	First_signal_lng(0),
	Last_signal_lng(0),
	Cooldown_End_lng(0),
	Last_detection_time_lng(0) {
	pinMode(Soundboard_gpio, INPUT);
}

// Detect a single sound event
bool rgSoundSensor::Detect(void) {
	bool retval_bool = false;

	// detect states changes at the sound board output
	// and debounce these signals: 1 clap of the hands triggers 11 LOW drops at SOUNDBOARD_GPIO during 30 ms
	unsigned long now = millis();

	if (Cooldown_End_lng && now < Cooldown_End_lng)
		return false;

	bool sensor_bool = digitalRead(Soundboard_gpio);
	if (sensor_bool != Last_sensor_bool) {
		// the sound board output has changed
		Last_sensor_bool = sensor_bool;
		if (sensor_bool == LOW) {
		// a sound is detected
		if ((First_signal_lng == 0) || (now - First_signal_lng > SILENCE_MIN_DURATION)) {
			// init or timeout
			First_signal_lng = now;
			Cooldown_End_lng = 0;
		}
		Last_signal_lng = now;
		//Serial.printf("%lu Sensor=%d First=%lu Last=%lu Dur=%lu\n", now, sensor_bool, First_signal_lng, Last_signal_lng, Last_signal_lng - First_signal_lng);
		}
	}
	if (sensor_bool == HIGH
		&& Last_signal_lng != 0
		&& now - Last_signal_lng > SILENCE_MIN_DURATION
		&& Last_signal_lng - First_signal_lng < SOUND_MAX_DURATION
	) {
		First_signal_lng = 0;
		Last_signal_lng = 0;
		Cooldown_End_lng = now + COOLDOWN_DURATION;
		retval_bool = true;
	}
	return retval_bool;
}

// Check for double sound detection
bool rgSoundSensor::Check(void) {
	bool retval_bool = false;

	unsigned long now = millis();

	unsigned long interval_int = Last_detection_time_lng ? now - Last_detection_time_lng : 0;
	if (interval_int > DOUBLE_MAX_INTERVAL)
		Last_detection_time_lng = 0;

	if (Detect()) {
		//Serial.printf("%lu Last=%lu Inter=%lu\n", now, Last_detection_time_lng, interval_int);
		if (!Last_detection_time_lng)
		Last_detection_time_lng = now; // 1st detection
		else {
			if (interval_int > DOUBLE_MIN_INTERVAL && interval_int < DOUBLE_MAX_INTERVAL)
				retval_bool = true;
			Last_detection_time_lng = 0;
		}
	}
	return retval_bool;
}
