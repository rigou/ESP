/* rgDigit.cpp
** 25-02-2012
*/

#include "rgDigit.h"

#define set_segment_pin(x,y) digitalWrite(Pins_byt[(x)], !(boolean)(y))

// printable characters
const char rgDigit::ValidChars[] = " -=0123456789ABCDEF";

// bit 7: SEGP; bit 6: SEGA, bit 5: SEGB, ...
const byte rgDigit::SegCodes[] = {
	B01001001, // undefined char
	B00000000, // space, all off
	B00000001, // -
	B00001001, // =
	B01111110, // 0
	B00110000, // 1
	B01101101, // 2
	B01111001, // 3
	B00110011, // 4
	B01011011, // 5
	B01011111, // 6
	B01110000, // 7
	B01111111, // 8
	B01111011, // 9
	B01110111, // A
	B00011111, // b
	B01001110, // C
	B00111101, // d
	B01001111, // E
	B01000111  // F
};

rgDigit::rgDigit() {
	// You must call Init() if you use this constructor
}

rgDigit::rgDigit(int common_pin_int, int segment_pin_int) {
	Init(common_pin_int, segment_pin_int);
}


/*
** Public interface -----------------------------------------------------------
*/

// common_pin_int : Arduino pin connected to digit's COMMON. COMMON enables/disables the digit.
// segment_pin_int : SEGP (decimal point)
// segment_pin_int +1 : SEGA (segment a)
// ...
// segment_pin_int +7 : SEGG (segment g)
void rgDigit::Init(int common_pin_int, int segment_pin_int) {
	SetValue(' ');
	SetDot(false);

	// Arduino pins used to control the segments
	for (int idx_int = 0; idx_int < DIGITPINS; idx_int++) {
		Pins_byt[idx_int] = segment_pin_int + idx_int;
		pinMode(Pins_byt[idx_int], OUTPUT);
		set_segment_pin(idx_int, LOW); // turn off segment
	}

	// Arduino pin used to enable/disable the digit
	PinCommon_byt = common_pin_int;
	pinMode(PinCommon_byt, OUTPUT);
	Enable(false); // turn off digit
}

void rgDigit::Enable(boolean state_bool) {
	digitalWrite(PinCommon_byt, state_bool);
}

void rgDigit::SetValue(char value_chr) {
	int idx_int = 0; // default = undefined char
	const char *value_ptr = strchr(ValidChars, toupper(value_chr));
	if (value_ptr)
		idx_int = value_ptr - ValidChars +1;
	
	/* Debug
	char line_str[80];
	sprintf(line_str, "SetValue(%c) idx %d", value_chr, idx_int);
	Serial.println(line_str);
	*/
	SegValue_byt = SegCodes[idx_int] ;
}

void rgDigit::SetDot(boolean state_bool) {
	Dot_bool = state_bool;
}

void rgDigit::Refresh() {
	byte segval_byt = SegValue_byt;
	if (Dot_bool)
		segval_byt |= 0x80;
	if (segval_byt == 0x00) {
		// all segments off
		// Optimization: no need to loop over all segments:
		// just turn off last segment, if any
		turn_segment(0, false);
	}
	else {
		// for each bit of SegValue_byt
		for (int idx_int = 0; idx_int < 8; idx_int++) {
			turn_segment(idx_int, segval_byt & 0x80);  // turn on/off segment
			segval_byt = segval_byt << 1;
			delayMicroseconds(100);
		}
	}
}

/*
** Private implementation -----------------------------------------------------
*/

void rgDigit::turn_segment(byte segment_byt, boolean state_bool) {
	static byte last_high_byt = 0xFF;
	if  (last_high_byt != 0xFF) {
		set_segment_pin(last_high_byt, LOW);
		last_high_byt = 0xFF;
	}
	if (state_bool) {
		set_segment_pin(segment_byt, HIGH);
		last_high_byt = segment_byt;
	}
}



