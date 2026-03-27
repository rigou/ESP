/* rgDigits.cpp
** 26-02-2012
*/

#include "rgDigits.h"

// common_pin_int : Arduino pin connected to digit #1 (rightmost) COMMON. COMMON enables/disables the digit.
// common_pin_int + 1: Arduino pin connected to digit #2 COMMON.
// ..
// segment_pin_int : SEGP (decimal point)
// segment_pin_int +1 : SEGA (segment a)
// ...
// segment_pin_int +7 : SEGG (segment g)
rgDigits::rgDigits(int common_pin_int, int segment_pin_int) {
	for (int idx_int=0; idx_int < NDIGITS; idx_int++) {
		Digits_obj[idx_int].Init(common_pin_int + idx_int, segment_pin_int);
		Digits_obj[idx_int].SetDot(false);
		Digits_obj[idx_int].Enable(false);
	}
}


/*
** Public interface -----------------------------------------------------------
*/

void rgDigits::SetValue(char *value_str) {
	int length_int = strlen(value_str);
	for (int idx_int = 0; idx_int < NDIGITS; idx_int++) {
		if (idx_int < length_int)
			Digits_obj[idx_int].SetValue(value_str[length_int - idx_int - 1]);
		else
			Digits_obj[idx_int].SetValue(' ');
	}
}

// refresh digits 20 times / second
void rgDigits::Refresh() {
	static unsigned long last_time_ulng = 0;
	unsigned long time_ulng = millis();
	
	//if (time_ulng > last_time_ulng + 50) { // else nop
		for (int idx_int = 0; idx_int < NDIGITS; idx_int++) {
			Digits_obj[idx_int].Enable(true);
			Digits_obj[idx_int].Refresh();
			Digits_obj[idx_int].Enable(false);
		}
		last_time_ulng = time_ulng;
	//}
}

/*
** Private implementation -----------------------------------------------------
*/

