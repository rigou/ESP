/* rgDigit.h
** 26-02-2012
*/
#ifndef rgDigit_h
#define rgDigit_h
#include <Arduino.h>

// Number of pins used to control the segments and the decimal point
#define DIGITPINS 8

class rgDigit {
	private:
		byte PinCommon_byt; // Arduino pin connected to digit's COMMON 
		byte Pins_byt[DIGITPINS]; // Arduino pins used to control the segments
		static const byte SegCodes[];
		static const char ValidChars[];
		byte SegValue_byt;
		boolean Dot_bool;
		
		void turn_segment(byte, boolean);

	public:
		rgDigit();
		rgDigit(int, int);
		void Init(int, int);
		void Enable(boolean);
		void SetValue(char);
		void SetDot(boolean);
		void Refresh();
};

#endif
