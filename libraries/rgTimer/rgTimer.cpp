/* rgTimer.cpp
** 08-01-2012
*/

#include "rgTimer.h"

// id_int timer ID [0, TIMER_COUNT-1], required if command_int=TIMER_GET or TIMER_SET
// command_int TIMER_RESET, TIMER_GET TIMER_SET
// delay_lng timer duration in milliseconds, required if command_int=TIMER_SET
// return value: if command_int=TIMER_GET:
//	<0: time left until expiration of delay_lng, in milliseconds
//	>=0: time since expiration of delay_lng, in milliseconds
// always 0 if command_int=TIMER_SET
long Timer(int id_int, int command_int, long delay_ulng) {
	long retval_lng=0L;
	static unsigned long expiration_ulng[TIMER_COUNT]; // time of expiration in milliseconds
	unsigned long now_ulng=millis();
	
	switch (command_int) {
		
		case TIMER_RESET:
			// reset all timers, required before using other commands
			for (id_int=0; id_int<TIMER_COUNT; id_int++)
				expiration_ulng[id_int]=now_ulng;
			break;
			
		case TIMER_SET:
			expiration_ulng[id_int]=now_ulng + (unsigned long)delay_ulng;
			break;
		
		case TIMER_GET:
			if (now_ulng >= expiration_ulng[id_int]) 
				retval_lng=now_ulng - expiration_ulng[id_int];
			else
				retval_lng= - (long) (expiration_ulng[id_int] - now_ulng);
			break;
	}
	return retval_lng;
}
