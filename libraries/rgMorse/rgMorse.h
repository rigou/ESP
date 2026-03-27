/* rgMorse.h - Morse decoder
** 19-01-2012
*/

/*
    short mark, dot or 'dit' (·) -- 'dot duration' is one unit long (100 ms)
    longer mark, dash or 'dah' (-) -- three units long (300 ms)
    inter-element gap between the dots and dashes within a character -- one dot duration or one unit long (100 ms)
    short gap (between letters) -- three units long (300 ms)
    medium gap (between words) -- seven units long[19] (700 ms)
*/


#ifndef rgMorse_h
#define rgMorse_h
#include <Arduino.h>

class rgMorse {
	private:
		int Inputpin_int; // for analog read
		int Signal_int; // average value of samples in Events_int[]
		int Ambient_int; // mean ambient signal value
		
		static const int CLOCKPERIOD;
		static const int DITTIME;
		static const int EVENTS;
		static const int THRESHOLD;
		int *Events_int;
		unsigned long *Times_ulng;
		int Eventidx_int; // last index used
		char Character_char;
		char *Code_str[7]; // Morse code, order by length
		char Morse_str[10];

		boolean computeSignal(int);
		void addEvent(int, unsigned long);
		char decodeCharacter();
		boolean charComplete();
		char morse2alpha(char *) ;
	
	public:
		rgMorse(const int);
		~rgMorse();
		void Reset();
		boolean Clock();
		boolean Signal();
		int Sigval();
		int Ambient();
		char *Morse();
		char Character();
};
#endif
