/* arduinotx_led.ino - Led manager
** 05-03-2013
** 05-06-2013  SetCode() array in PROGMEM
*/

/* Copyright (C) 2013 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include "arduinotx_led.h"
#include "arduinotx_lib.h"

/*
** Public -----------------------------------------------------------------
*/

ArduinotxLed::ArduinotxLed(byte pin_byt) {
	Current_char = '\0';
	OutPin_byt = pin_byt;
	for (byte idx_byt = 0; idx_byt < 11; idx_byt++)
		Flash_byt[idx_byt] = 10;
	pinMode(pin_byt, OUTPUT);
}

// Allocate Morse codes in PROGMEM
// |A.-|B-...|C-.-.|D-..|E.|F..-.|G--.|H....|I..|J.---|K-.-|L.-..|M--|N-.|O---|P.--.|Q--.-|R.-.|S...|T-|U..-|V...-|W.--|X-..-|Y-.--|Z--..|0-----|1.----|2..---|3...--|4....-|5.....|6-....|7--...|8---..|9----.|..-.-.-|?..--..|!-.-.--|,--..--|@-.--.-.|
const char Morse_C[] PROGMEM = "-.-."; const char Morse_D[] PROGMEM = "-.."; const char Morse_E[] PROGMEM = "."; const char Morse_I[] PROGMEM = ".."; const char Morse_T[] PROGMEM = "-"; 
const char Morse_0[] PROGMEM = "-----"; const char Morse_1[] PROGMEM = ".----"; const char Morse_2[] PROGMEM = "..---"; const char Morse_3[] PROGMEM = "...--"; const char Morse_4[] PROGMEM = "....-";
const char Morse_5[] PROGMEM = "....."; const char Morse_6[] PROGMEM = "-...."; const char Morse_7[] PROGMEM = "--..."; const char Morse_8[] PROGMEM = "---.."; const char Morse_9[] PROGMEM = "----.";
PGM_P MorseCodes_str[] PROGMEM = {
	Morse_C, Morse_D, Morse_E, Morse_I, Morse_T,
	Morse_0, Morse_1, Morse_2, Morse_3, Morse_4, Morse_5, Morse_6, Morse_7, Morse_8, Morse_9,
	NULL
};

// Set the character to display
// retrieve the morse sequence corresponding to given character
// and store it in the Flash_byt[] array
// returns 0=ok, 1=char not found
byte ArduinotxLed::SetCode(const char c_char) {
	byte retval_byt = 0;
	if (c_char != Current_char) {
		// seek position of c_char in alphabet_str
		const char *alphabet_str = "CDEIT0123456789";
		const char *c_ptr = strchr(alphabet_str, c_char);
		if (c_ptr) {
			// found
			char morse_str[7];
			int idx_int = c_ptr - alphabet_str;
			getProgmemStrArrayValue(morse_str, MorseCodes_str, idx_int, 7);
			aprintfln("%c idx=%d morse=%s", c_char, idx_int, morse_str);
			// convert morse to delays
			byte idx_byt = 0;
			char *pos_ptr=morse_str;
			while (*pos_ptr) {
				Flash_byt[idx_byt++] = *pos_ptr == '.' ? 20:60; // dot=200ms, dash=600ms
				Flash_byt[idx_byt++] = 20; // inter-element gap between the dots and dashes within a character=200 ms
				++pos_ptr;
			}
			Flash_byt[idx_byt - 1] = 100; // gap between letters=1000ms
			Flash_byt[idx_byt] = 0;
			Current_char = c_char;
			Flash(1); // reset
		}
		else
			retval_byt = 1;
	}
	return retval_byt;
}

// Flash led, public calls
// Call this method every few milliseconds to flash the Led according to current OutPin_byt[] timings
void ArduinotxLed::Flash() {
	Flash(0);
}

/*
** Private ----------------------------------
*/

// Flash led, private calls
// reset_byt 1=reset internal state, for private calls only
void ArduinotxLed::Flash(byte reset_byt) {
	static byte Flash_idx_byt = 0;
	static unsigned long Begin_ulng = 0;
	
	if (reset_byt) {
		Flash_idx_byt = 0;
		Begin_ulng = millis();
		digitalWrite(OutPin_byt, *Flash_byt ? HIGH:LOW);
	}
			
	byte pulse_byt=*(Flash_byt + Flash_idx_byt);
	if (pulse_byt) {
		unsigned long now_ulng = millis();
		if (now_ulng >= Begin_ulng + (10 * pulse_byt)) {
			++Flash_idx_byt;
			Begin_ulng = now_ulng;
			digitalWrite(OutPin_byt, Flash_idx_byt % 2 == 0 ? HIGH:LOW);
		}
	}
	else
		Flash_idx_byt = 0; // restart from 1st pulse
}


