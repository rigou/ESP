/* rgMotor.h
** 29-01-2012
*/
#ifndef rgMotor_h
#define rgMotor_h
#include <Arduino.h>

class rgMotor {
	private:
		// Arduino pins used to control the motor: 2,3,4,7-12
		int Power_int; // current power [-1000,+1000] : stop=0, forward>0, backward<0
		int PowerMin_int; // minimum power required to move the motor [0,+1000], default = 0
		int PowerMax_int; // maximum power applicable to the motor [0,+1000], default = 1000
		int In1Pin_int; // the Arduino output pin number connected to the IN1 (or IN3) input of the L298 motor driver
		int In2Pin_int; // the Arduino output pin number connected to the IN2 (or IN4) input of the L298 motor driver
		int EnablePin_int; // the Arduino output pin number connected to the ENA (or ENB) input of the L298 motor driver

	public:
		rgMotor(int, int, int);
		~rgMotor();
		void setPower(int);
		int Power();
		void setPowerMin(int);
		int PowerMin();
		void setPowerMax(int);
		int PowerMax();
};

#endif
