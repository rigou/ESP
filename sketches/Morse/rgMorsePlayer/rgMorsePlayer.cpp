/* rgMorsePlayer.cpp - Morse player
** 11-02-2012
*/

/*
    short mark, dot or 'dit' (·) -- 'dot duration' is one unit long (100 ms)
    longer mark, dash or 'dah' (-) -- three units long (300 ms)
    inter-element gap between the dots and dashes within a character -- one dot duration or one unit long (100 ms)
    short gap (between letters) -- three units long (300 ms)
    medium gap (between words) -- seven units long[19] (700 ms)
*/

#include "rgMorsePlayer.h"

const int rgMorsePlayer::DITTIME=100; // (ms) duration of a morse dot; dash duration >=  3*DITTIME

rgMorsePlayer::rgMorsePlayer()  {
	Code_str = "|  |A.-|B-...|C-.-.|D-..|E.|F..-.|G--.|H....|I..|J.---|K-.-|L.-..|M--|N-.|O---|P.--.|Q--.-|R.-.|S...|T-|U..-|V...-|W.--|X-..-|Y-.--|Z--..|0-----|1.----|2..---|3...--|4....-|5.....|6-....|7--...|8---..|9----.|..-.-.-|?..--..|!-.-.--|,--..--|@-.--.-.";
	Pin(8); // set default pin for audio output
}

/*
** Public interface
*/


void rgMorsePlayer::Pin(const int pin_int)  {
	Outputpin_int = pin_int;
	pinMode(Outputpin_int, OUTPUT);
}

void rgMorsePlayer::Play(char *text_str) {
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
boolean rgMorsePlayer::char2morse(char c_char, char *out_sequence_str){
	boolean retval_bool=true;
	memset(out_sequence_str, '\0', 8);
	char *key_str = "|."; 
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
	//Serial.print("rgMorsePlayer::char2morse() "); Serial.print(c_char); Serial.print(" : "); Serial.println(out_sequence_str);
	return retval_bool;
}

// play given morse sequence corresponding to a single character
// the sequence is composed of a series of dots and dashes, or of a single space
void rgMorsePlayer::play_morse(char *sequence_str) {
	if (*sequence_str ==  ' ')
		delay(7 * DITTIME); // medium gap (between words) 
	else {
		while (char m_char = *sequence_str++) {
			switch (m_char) {
				case '.':
					beep(DITTIME); // short mark, dot or 'dit' (·)
					break;
				case '-':
					beep(3 * DITTIME); // longer mark, dash or 'dah' (-)
					break;
			}
			delay(DITTIME); // inter-element gap between the dots and dashes within a character 
		}
		delay(3 * DITTIME); // short gap (between letters) 
	}
}

void rgMorsePlayer::beep(int delay_int) {
	while (delay_int-- > 0) {
		digitalWrite(Outputpin_int, HIGH);
		delayMicroseconds(500);
		digitalWrite(Outputpin_int, LOW);
		delayMicroseconds(500);
	}
}

