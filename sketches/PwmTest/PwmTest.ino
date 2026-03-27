/* PwmTest - This is an example for PCA9685 16 channel 12 bit PWM I²C driver
 *  2022-12-19
 */

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include <rgLed.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver Pwm_obj = Adafruit_PWMServoDriver(0x40);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver Pwm_obj = Adafruit_PWMServoDriver(0x40, Wire);

// 50 Hz is the standard PWM frequency for servomotors
// The best actual frequency obtained by adjusting setOscillatorFrequency (see below) is 50.20 Hz
const unsigned int PWM_FREQ=50;

/* The standard pulse width for servos ranges from 1000 to 2000 microseconds:
 * servos are supposed to move their arm to the 0° position when receiving a 1000 microseconds pulse, 
 * and move it to the 90° position when receiving a 2000 microseconds pulse.
 * However, many servos support or require a wider pulse range. HobbyKing's HXT500 servos, 
 * for instance, respond to a range of 720-2200 microseconds pulses, moving their arm by 110°.
 * Entering the widest pulse range corresponding to your servos will achieve maximal mechanical resolution. 
*/
unsigned int conv_microsec_to_duty_cycle(int microsec_int) {
	return (4096*microsec_int)/(1000000/PWM_FREQ);
}

rgLed Led_obj;

// give user some time to open the serial monitor
static void start_serial(void) {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	int count_int=0;
	Serial.println("");
	while (count_int<5) {
		Serial.printf("%c", 'A'+count_int++);
		delay(1000);
	}
	Serial.println("");
}

void setup() {
  	start_serial("");

	Pwm_obj.begin();
	/*
	* In theory the internal oscillator (clock) is 25MHz but it really isn't
	* that precise. You can 'calibrate' this by tweaking this number until
	* you get the PWM update frequency you're expecting!
	* The int.osc. for the PCA9685 chip is a range between about 23-27MHz and
	* is used for calculating things like writeMicroseconds()
	* Analog servos run at ~50 Hz updates, It is importaint to use an
	* oscilloscope in setting the int.osc frequency for the I2C PCA9685 chip.
	* 1) Attach the oscilloscope to one of the PWM signal pins and ground on
	*    the I2C PCA9685 chip you are setting the value for.
	* 2) Adjust setOscillatorFrequency() until the PWM update frequency is the
	*    expected value (50Hz for most ESCs)
	* Setting the value here is specific to each individual I2C PCA9685 chip and
	* affects the calculations for the PWM update frequency. 
	* Failure to correctly set the int.osc value will cause unexpected PWM results
	*/
	Pwm_obj.setOscillatorFrequency(25800000); // 27,000,000=47.97 Hz 25,920,000=49.80 Hz 25,800,000=50.20 Hz ; intermediate values give 49.80 or 50.20 Hz
	
	Pwm_obj.setPWMFreq(PWM_FREQ);  
	
	// if you want to really speed stuff up, you can go into 'fast 400khz I2C' mode
	// some i2c devices dont like this so much so if you're sharing the bus, watch
	// out for this!
	Wire.setClock(400000);

	// set a distinct fixed frequency for each pwm output
	// output_int 0 : 1000 microsec ... output_int 15 : 2005 microsec
	for (unsigned short output_int=0; output_int < 16; output_int++) {
		unsigned int pulse_len_int=1000+(output_int*67);
		Pwm_obj.setPWM(output_int, 0, conv_microsec_to_duty_cycle(pulse_len_int));
	}
}

void loop() {
	static unsigned int count_int=0;
	Serial.printf("%d,",++count_int);
	if (count_int%10==0)
		Serial.println("");

	Led_obj.ErrLed(!(count_int%2));

	Led_obj.Heartbeat(2);
}
