/* rgKeyboard.cpp
** 22-02-2012
** 11-03-2012 constructor, Init()
** 11-10-2012 Key()
*/

#include "rgKeyboard.h"

rgKeyboard::rgKeyboard() {
	KbMap_str[0][0] = "UTILITY"; KbMap_str[0][1] = "START"; KbMap_str[0][2] = "STOP"; KbMap_str[0][3] = "CLR"; KbMap_str[0][4] = "0"; KbMap_str[0][5] = "ENT"; 
	KbMap_str[1][0] = "PRINT"; KbMap_str[1][1] = "FEED"; KbMap_str[1][2] = "Down"; KbMap_str[1][3] = "1"; KbMap_str[1][4] = "2"; KbMap_str[1][5] = "3"; 
	KbMap_str[2][0] = "ID"; KbMap_str[2][1] = "POST"; KbMap_str[2][2] = "Up"; KbMap_str[2][3] = "4"; KbMap_str[2][4] = "5"; KbMap_str[2][5] = "6"; 
	KbMap_str[3][0] = "VC"; KbMap_str[3][1] = "FVC"; KbMap_str[3][2] = "MVV"; KbMap_str[3][3] = "7"; KbMap_str[3][4] = "8"; KbMap_str[3][5] = "9"; 
	
	KeyTime_ulng = 0; // time of keydown and keyup detection, 0=all keys are up > 50 ms
	Keydown_bool = false; // true=keydown detected, now counting how long it is being pressed; false=no key is being pressed
}

/*
** Public interface -----------------------------------------------------------
*/

void rgKeyboard::Init() {
	int rowpins_int[] = {32, 34, 36,38};
	int colpins_int[] = {31, 33, 35, 37, 39, 30};
	Init(rowpins_int, colpins_int);
}

void rgKeyboard::Init(int *rowpins_int, int *colpins_int) {
	int idx_int = 0; 
	for (idx_int = 0; idx_int < NCOLS; idx_int++) {
		KbCols_int[idx_int] = colpins_int[idx_int];
		pinMode(KbCols_int[idx_int], INPUT);
		digitalWrite(KbCols_int[idx_int], HIGH); // enable internal 20K pullup resistor
	}
	for (idx_int = 0; idx_int < NROWS; idx_int++) {
		KbRows_int[idx_int] = rowpins_int[idx_int];
		pinMode(KbRows_int[idx_int], OUTPUT);
	}
	
	enable_keys(true); // power up the keyboard grid
}
	
// scan the keyboard for a key hit
// return value; true=the string value of the key that was pressed for > 100ms, null=no key pressed
char *rgKeyboard::Key() {
	char *retval_str = NULL;
	static boolean returned_bool = false; // true when the pressed key value has been returned and the key has not yet been released 
	unsigned long now_ulng = millis();
	boolean key_down_bool = scan_keys();
	if (key_down_bool) {
		if (!Keydown_bool) {
			// first detection of key down
			KeyTime_ulng = now_ulng;
			Keydown_bool = true;
		}
		else if (now_ulng - KeyTime_ulng > 100 && !returned_bool ) {
			// key has been pressed long enough
			retval_str = KbMap_str[KeyPressed_int[0]][KeyPressed_int[1]];
			returned_bool = true;
		}
	}
	else {
		if (Keydown_bool) {
			// key up detected
			KeyTime_ulng = 0;
			Keydown_bool = false;
			returned_bool = false;
		}
	}
	return retval_str;
}

/*
** Private implementation -----------------------------------------------------
*/

// power up/down the keyboard grid
void rgKeyboard::enable_keys(boolean state_bool) {
	for (int idx_int = 0; idx_int < NROWS; idx_int++)
		digitalWrite(KbRows_int[idx_int], state_bool);
}

// Scan the keyboard in search of a pressed key:
// - all input pins are set HIGH by default by the internal 20K pullup resistors
// - when a key is pressed, the corresponding input turns LOW
// Store coordinates of the pressed key in KeyPressed_int[]
// Return value; true=a key is down, false=all keys up
boolean rgKeyboard::scan_keys() {
	boolean retval_bool = false;
	
	// for each row
	for (int row_int = 0; row_int < NROWS; row_int++) {
		digitalWrite(KbRows_int[row_int], LOW);
		// for each column
		for (int col_int = 0; col_int < NCOLS; col_int++) {
			if (digitalRead(KbCols_int[col_int]) == LOW) {
				// key pressed
				KeyPressed_int[0] = row_int;
				KeyPressed_int[1] = col_int;
				retval_bool = true;
				break;
			}
		}
		digitalWrite(KbRows_int[row_int], HIGH);
		if (retval_bool)
			break;
	}
	
	return retval_bool;
}


