/* rgMotor.cpp
** 29-01-2012
*/

#include "rgMotor.h"

rgMotor::rgMotor(int in1_int, int in2_int, int enable_int) {
	// Arduino pins used to control the motor: 2,3,4,7-12
	In1Pin_int = in1_int; // the Arduino output pin number connected to the IN1 (or IN3) input of the L298 motor driver
	In2Pin_int = in2_int; // the Arduino output pin number connected to the IN2 (or IN4) input of the L298 motor driver
	EnablePin_int = enable_int; // the Arduino output pin number connected to the ENA (or ENB) input of the L298 motor driver
	
	pinMode(In1Pin_int, OUTPUT);
	pinMode(In2Pin_int, OUTPUT);
	pinMode(EnablePin_int, OUTPUT);
	
	Power_int = 0; // [-1000,+1000] : stop=0, forward>0, backward<0
	setPower(0);
	setPowerMin(0);
	setPowerMax(1000);
}

rgMotor::~rgMotor() {
	setPower(0);
}

// minimum power required to move the motor [0,+1000], default = 0
int rgMotor::PowerMin() {
	return PowerMin_int;
}

void rgMotor::setPowerMin(int newpower_int) {
	PowerMin_int = constrain(newpower_int, 0, 1000);
}

// maximum power applicable to the motor [0,+1000], default = 1000
int rgMotor::PowerMax() {
	return PowerMax_int;
}

void rgMotor::setPowerMax(int newpower_int) {
	PowerMax_int = constrain(newpower_int, 0, 1000);
}

// return current power: [-1000,+1000] : stop=0, forward>0, backward<0
int rgMotor::Power() {
	return Power_int;
}

void rgMotor::setPower(int newpower_int) {
	newpower_int = constrain(newpower_int, -1000, 1000);
	if (newpower_int == 0) {
		// set null power
		digitalWrite(In1Pin_int, LOW);
		digitalWrite(In2Pin_int, LOW);
		digitalWrite(EnablePin_int, LOW);
	}
	else if (newpower_int != Power_int) { // else unchanged power
		if  (newpower_int > 0) {
			// set direction: forward
			if (Power_int <= 0) {
				// change direction
				digitalWrite(In1Pin_int, HIGH);
				digitalWrite(In2Pin_int, LOW);
			}
		}
		else {
			// set direction: reverse
			if (Power_int >= 0) {
				// change direction
				digitalWrite(In1Pin_int, LOW);
				digitalWrite(In2Pin_int, HIGH);
			}
		}
		// set power
		int powerval_int = map(abs(newpower_int), 0, 1000, PowerMin_int, PowerMax_int);
		analogWrite(EnablePin_int, map(powerval_int, 0, 1000, 0, 255));
	}
	Power_int = newpower_int; // store current power
}


