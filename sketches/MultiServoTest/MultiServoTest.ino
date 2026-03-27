/*
 * ESP32 Servo Example Using Arduino ESP32 Servo Library
 * John K. Bennett
 * March, 2017
 * 
 * This sketch uses the Arduino ESP32 Servo Library to sweep 4 servos in sequence.
 * 
 * Different servos require different pulse widths to vary servo angle, but the range is 
 * an approximately 500-2500 microsecond pulse every 20ms (50Hz). In general, hobbyist servos
 * sweep 180 degrees, so the lowest number in the published range for a particular servo
 * represents an angle of 0 degrees, the middle of the range represents 90 degrees, and the top
 * of the range represents 180 degrees. So for example, if the range is 1000us to 2000us,
 * 1000us would equal an angle of 0, 1500us would equal 90 degrees, and 2000us would equal 1800
 * degrees.
 * 
 * Circuit:
 * Servo motors have three wires: power, ground, and signal. The power wire is typically red,
 * the ground wire is typically black or brown, and the signal wire is typically yellow,
 * orange or white. Since the ESP32 can supply limited current at only 3.3V, and servos draw
 * considerable power, we will connect servo power to the VBat pin of the ESP32 (located
 * near the USB connector). THIS IS ONLY APPROPRIATE FOR SMALL SERVOS. 
 * 
 * We could also connect servo power to a separate external
 * power source (as long as we connect all of the grounds (ESP32, servo, and external power).
 * In this example, we just connect ESP32 ground to servo ground. The servo signal pins
 * connect to any available GPIO pins on the ESP32 (in this example, we use pins
 * 22, 19, 23, & 18).
 * 
 * In this example, we assume four Tower Pro SG90 small servos.
 * The published min and max for this servo are 500 and 2400, respectively.
 * These values actually drive the servos a little past 0 and 180, so
 * if you are particular, adjust the min and max values to match your needs.
 * Experimentally, 550 and 2350 are pretty close to 0 and 180.
 * 
 * Useful Defaults:
	default min pulse width for attach(): 544us
	default max pulse width for attach(): 2400us
	default timer width 16 (if timer width is not set)
	default pulse width 1500us (servos are initialized with this value)
	MINIMUM pulse with: 500us
	MAXIMUM pulse with: 2500us
	MAXIMUM number of servos: 16 (this is the number of PWM channels in the ESP32)
 */

#include <ESP32Servo.h>


// Published values for SG90 servos; adjust if needed
const int minUs = 500; // 1000
const int maxUs = 2500; // 2000

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42
// for the ESP32-S3 the GPIO pins are 1-21,35-45,47-48
// for the ESP32-C3 the GPIO pins are 1-10,18-21
const byte NSERVOS=6;
byte ServoPin[]={13, 12, 14, 27, 32, 33}; // 6 channels
//byte ServoPin[]={13, 12, 14, 27, 32, 33, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26}; // 16 channels
Servo Servo_obj[NSERVOS];

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	// give user some time to open the serial monitor
	Serial.println("");
	int idx = 0;
	while (idx < 12) {
		Serial.printf("%c", 'A' + idx++);
		delay(500);
	}

	// Allow allocation of all timers
	for (idx=0; idx<4; idx++)
		ESP32PWM::allocateTimer(idx);

	for (idx=0; idx<NSERVOS; idx++) {
		Servo_obj[idx].setPeriodHertz(50);
		Servo_obj[idx].attach(ServoPin[idx], minUs, maxUs);
	}
}

// Servo.write() takes 82 micros/servo at 40 MHz, 13 micros/servo at 240 MHz (it is proportional to 1/cpu frequency)
void loop() {
	int idx=0;
	int pos=0;
	unsigned long begin_time=0;
	for (pos = 180; pos >= 0; pos--) { // sweep from 0 degrees to 180 degrees
		begin_time=micros();
		for (idx=0; idx<NSERVOS; idx++) {
			Servo_obj[idx].write(pos);
		}
		//  6 servos : 458-472 micros at cpu 40 MHz ; 77-80 micros at cpu 240 MHz
		// 16 servos : 1271-1308 micros at cpu 40 MHz ; 206-212 micros at cpu 240 MHz
		Serial.println(micros()-begin_time);
		delay(15);
	}
	delay(1000);
}

