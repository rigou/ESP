#pragma once
#include <Arduino.h>

class rgSoundSensor {
private:
	int Soundboard_gpio;
	bool Last_sensor_bool;
	unsigned long First_signal_lng;
	unsigned long Last_signal_lng;
	unsigned long Cooldown_End_lng;
	unsigned long Last_detection_time_lng;

	const int SOUND_MAX_DURATION = 100;      // ms
	const int SILENCE_MIN_DURATION = 100;    // ms
	const int COOLDOWN_DURATION = 500;       // ms
	const int DOUBLE_MIN_INTERVAL = 500;     // ms
	const int DOUBLE_MAX_INTERVAL = 3000;    // ms

public:
	// Constructor
	rgSoundSensor(int gpio);

	// Method to detect a single sound event
	bool Detect(void);

	// Method to check for double sound detection
	bool Check(void);
};
