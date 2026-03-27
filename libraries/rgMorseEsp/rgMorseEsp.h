#pragma once

#define TONE_GPIO	4 // labeled D2 on ESP8266
#define TONE_FREQ	440 // sound freq for cpu speed=240 MHz, lower cpu speed gives lower freq

class rgMorseEsp {
	private:
		static const int DITTIME; // (ms) duration of a morse dot; dash duration >=  3*DITTIME
		const char *Code_str; // Morse code
		bool char2morse(char, char *);
		void play_morse(char *);
		void beep(int);
	
	public:
		rgMorseEsp();
		void Pin(const int);
		void Play(char *);
};
