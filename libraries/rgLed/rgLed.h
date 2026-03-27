/* rgLed.h - simple runled portable implementation
*  if this library is used in a project along with the rgWiFi library, 
*  then include rgLed.h AFTER rgWiFi.h
*
*  LED_BUILTIN GPIO Number:
*  	WEMOS LOLIN32 Lite: 22 (GPIO 22 has inverted logic and is not suitable for pwm) ARDUINO_LOLIN32_LITE
*  	ESP32 Dev Module: 2 (LED_BUILTIN is undefined)
*  	ESP8266 NodeMCU 1.0 (ESP-12E module): 2 (inverted logic)
*  	ESP01S: 2
*  	ESP12F: 2
*	ESP32-C3-zero: 10
*  	Arduinos: 13
*/

#pragma once

#include <Arduino.h>

#define LEDLIB_NAME	"rgLed" // spaces not permitted
#define LEDLIB_VERSION	"v2.0.1"

// default parameters
#ifndef LED_BUILTIN
	#if ARDUINO_LOLIN32_LITE
		#define LED_BUILTIN 22
	#elif ARDUINO_ESP32C3_DEV
		#define LED_BUILTIN 10
	#else
		#define LED_BUILTIN 2
	#endif
#endif

#define LED_GPIO LED_BUILTIN	// this activity led provides a visual feedback of the loop() function, see com_RunLed()

// SOC-specific parameters
#if ESP32
	#if ARDUINO_LOLIN32_LITE
		#define LED_INVERTED true
	#else
		#define LED_INVERTED false
	#endif
#elif ESP8266
	#define LED_INVERTED true
#else // Arduinos
	#define LED_INVERTED false
#endif
// you can override these hard-coded parameters when calling the constructor

class rgLed {
	public:
		rgLed(
			unsigned short runled_gpio_int=LED_GPIO, 
			bool runled_inverted_bool=LED_INVERTED
		);
		unsigned short getGpio(void);
		void Blink(unsigned int period, unsigned int time_on, bool restart=false);
		void Flash(unsigned int time_on=0);
		void Heartbeat(void);
		void SetLed(bool state_bool);
		void Delay(unsigned long millisec_lng);

	private:
		// initialized by the constructor:
		unsigned short mGpio_int=0;
		bool mInverted_bool=0;

		unsigned long mBegin_time=0;
		unsigned long mEnd_time=0;
		bool mLed_state=LOW;
};
