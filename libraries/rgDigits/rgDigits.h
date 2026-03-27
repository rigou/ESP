/* rgDigits.h
** 26-02-2012
*/
#ifndef rgDigits_h
#define rgDigits_h
#include <Arduino.h>

// Total number of digits
#define NDIGITS 2

#include "rgDigit.h"

class rgDigits {
	private:
		rgDigit Digits_obj[NDIGITS];
	
	public:
		rgDigits(int, int);
		void SetValue(char *);
		void Refresh();
};

#endif
