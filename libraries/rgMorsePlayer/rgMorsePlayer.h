/* rgMorsePlayer.h - Morse player
** 11-02-2012
*/

/*
    short mark, dot or 'dit' (·) -- 'dot duration' is one unit long (100 ms)
    longer mark, dash or 'dah' (-) -- three units long (300 ms)
    inter-element gap between the dots and dashes within a character -- one dot duration or one unit long (100 ms)
    short gap (between letters) -- three units long (300 ms)
    medium gap (between words) -- seven units long[19] (700 ms)
*/


#ifndef rgMorsePlayer_h
#define rgMorsePlayer_h
#include <Arduino.h>

class rgMorsePlayer {
	private:
		static const int DITTIME; // (ms) duration of a morse dot; dash duration >=  3*DITTIME
		const char *Code_str; // Morse code
		int Outputpin_int; // for audio tone output
		boolean char2morse(char, char *);
		void play_morse(char *);
		void beep(int);
	
	public:
		rgMorsePlayer();
		void Pin(const int);
		void Play(char *);
};
#endif
