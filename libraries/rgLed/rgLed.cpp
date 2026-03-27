/* rgLed.cpp - simple runled portable implementation
** if this library is used in a project along with the rgWiFi library, 
** then include rgLed.h AFTER rgWiFi.h
** 2023-04-21
** 2025-03-18 removed ErrLed support
** 2025-06-11 added Blink(), Flash()
*/

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

#include "rgLed.h"

#ifdef RGWIFI_OTA_ENABLED
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#endif

// if the object is constructed with gpio_int = 0 then all i/o methods will return instantly without performing any i/o
rgLed::rgLed(unsigned short gpio_int, bool inverted_bool) {
	mGpio_int=gpio_int;
	mInverted_bool=inverted_bool;
	if (mGpio_int)
		pinMode(mGpio_int, OUTPUT);
}

unsigned short rgLed::getGpio(void) {
	return mGpio_int;
}

// Blink the led, non blocking call : call it repeatedly
// this function can deal only with a single led
// period : time the led is on + time the led is off, in ms
// time_on : time the led is on, in ms
// restart : true=start a new period and turn on the led immediately else simply refresh its state
void rgLed::Blink(unsigned int period, unsigned int time_on, bool restart) {
	//dbprintf("Blink(%d, %d, %d)\n", period, time_on, restart);
	if (mGpio_int) {
		unsigned long time_now=millis();
		time_on=min(time_on, period);

		if (restart)
			mBegin_time=0;

		if (time_now > mBegin_time+period && mLed_state==LOW) {
			mBegin_time=time_now; // start a new period
			SetLed(HIGH);
			mLed_state=HIGH;
			trprintf("Blink gpio %d: now %lu, begin %lu, period %d : HIGH\n", mGpio_int, time_now, mBegin_time, period);
		}
		if (time_now > mBegin_time+time_on && mLed_state==HIGH) {
			SetLed(LOW);
			mLed_state=LOW;
			trprintf("Blink gpio %d: now %lu, begin %lu, time_on %d : LOW\n", mGpio_int, time_now, mBegin_time, time_on);
		}
	}
}

// Flash the led once, non blocking call : call it repeatedly
// time_on : time the led is on, in ms, optional
// if time_on is given then turn on the led immediately else simply refresh its state
void rgLed::Flash(unsigned int time_on) {
	if (mGpio_int) {
		unsigned long time_now=millis();
		if (time_on && mLed_state==LOW) {
			mEnd_time=time_now+time_on;
			SetLed(HIGH);
			mLed_state=HIGH;
		}
		else if (time_now > mEnd_time && mLed_state==HIGH) {
			SetLed(LOW);
			mLed_state=LOW;
		}
	}
}

// Flash the led for 50 ms every 2 s, non blocking call
// provided for backward compatibility
void rgLed::Heartbeat(void) {
	if (mGpio_int)
		Blink(2000, 50);
}

void rgLed::SetLed(bool state_bool) {
	if (mGpio_int)
		digitalWrite(mGpio_int, mInverted_bool?!state_bool:state_bool);
}


#ifdef RGWIFI_OTA_ENABLED
// same as delay() but OTA friendly
void rgLed::Delay(unsigned long millisec_lng) {
	rgwifi_OtaDelay(millisec_lng);
}
#else
// same as delay(), blocking but interruption friendly
void rgLed::Delay(unsigned long millisec_lng) {
	unsigned long start_millis=millis();
	while (millis()-start_millis < millisec_lng)
		yield(); // required for ESP8266, does NOP for other SOCs
}
#endif

