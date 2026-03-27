/* rgKeyboard.h
** 10-03-2012
*/
#ifndef rgKeyboard_h
#define rgKeyboard_h
#include <Arduino.h>

class rgKeyboard {
	private:
		// Arduino pins used to control the keyboard
		static const int NROWS = 4;
		static const int NCOLS = 6;
		int KbRows_int[NROWS];
		int KbCols_int[NCOLS];
		char *KbMap_str[NROWS][NCOLS]; // keyboard mapping
		int KeyPressed_int[2]; // row, col of last key pressed
		unsigned long KeyTime_ulng; // time of keydown and keyup detection, 0=all keys are up > 50 ms
		boolean Keydown_bool; // true=keydown detected, now counting how long it is being pressed; false=no key is being pressed
		void enable_keys(boolean);
		boolean scan_keys();

	public:
		rgKeyboard();
		void Init();
		void Init(int *, int *);
		char *Key();
};

#endif
