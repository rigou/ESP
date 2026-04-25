/* rgMorseEsp.cpp - Morse player for ESP-32
** 11-02-2012 original rgMorsePlayer for Arduino ARM
** 2023-04-04 ported to ESP-32
** 2023-04-14 beep()
**
    short mark, dot or 'dit' (.) -- 'dot duration' is one unit long (100 ms)
    longer mark, dash or 'dah' (-) -- three units long (300 ms)
    inter-element gap between the dots and dashes within a character -- one dot duration or one unit long (100 ms)
    short gap (between letters) -- three units long (300 ms)
    medium gap (between words) -- seven units long[19] (700 ms)
*/

#include <Arduino.h>
#include <memory.h>
#include "rgMorseEsp.h"
#include "rgCarrier.h"

const int rgMorseEsp::DITTIME=100; // (ms) duration of a morse dot; dash duration >=  3*DITTIME

rgMorseEsp::rgMorseEsp()  {
	Code_str = "|  |A.-|B-...|C-.-.|D-..|E.|F..-.|G--.|H....|I..|J.---|K-.-|L.-..|M--|N-.|O---|P.--.|Q--.-|R.-.|S...|T-|U..-|V...-|W.--|X-..-|Y-.--|Z--..|0-----|1.----|2..---|3...--|4....-|5.....|6-....|7--...|8---..|9----.|..-.-.-|?..--..|!-.-.--|,--..--|@-.--.-.";
}

/*
** Public interface
*/

void rgMorseEsp::Play(char *text_str) {
	for (int idx_int=0; text_str[idx_int]; idx_int++) {
		char sequence_str[8];
		if (char2morse(text_str[idx_int], sequence_str)) // else ignore undefined char
			play_morse(sequence_str);
	}
}

/*
** Private implementation
*/

// retrieve the morse sequence corresponding to given character
// and store it in given user-defined buffer
// returns true=ok, false=char not found
bool rgMorseEsp::char2morse(char c_char, char *out_sequence_str){
	bool retval_bool=true;
	memset(out_sequence_str, '\0', 8);
	char key_str[3] = {'|','.', '\0'};
	key_str[1] = toupper(c_char);
	char *pos_ptr=strstr(Code_str, key_str);
	if (pos_ptr != NULL) {
		// found
		int idx_int = 0;
		pos_ptr += 2;
		while (*pos_ptr != '|')
			out_sequence_str[idx_int++] = *pos_ptr++;
	}
	else
		retval_bool = false;
	//Serial.print("rgMorseEsp::char2morse() "); Serial.print(c_char); Serial.print(" : "); Serial.println(out_sequence_str);
	return retval_bool;
}

// play given morse sequence corresponding to a single character
// the sequence is composed of a series of dots and dashes, or of a single space
void rgMorseEsp::play_morse(char *sequence_str) {
	if (*sequence_str ==  ' ')
		delay(10 * DITTIME); // medium gap (between words) (standard value is 7)
	else {
		while (char m_char = *sequence_str++) {
			switch (m_char) {
				case '.':
					beep(DITTIME); // short mark, dot or 'dit' (.)
					break;
				case '-':
					beep(4 * DITTIME); // longer mark, dash or 'dah' (-) (standard value is 3)
					break;
			}
			delay(DITTIME); // inter-element gap between the dots and dashes within a character 
		}
		delay(4 * DITTIME); // short gap (between letters) (standard value is 3)
	}
}

// delay_int milliseconds
void rgMorseEsp::beep(int delay_int) {
	rgcarrier_tone(TONE_GPIO, TONE_FREQ, delay_int);
}

