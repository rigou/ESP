/* rgUI.h
* 12-02-2012
*/
#ifndef rgUI_h
#define rgUI_h
#include <Arduino.h>

class rgUI {
	private:
		boolean Init_bool;
		void print_line(int);
		void print_box_text(char *, int);
		
	public:
		rgUI();
		void Init();
		void Title(char *, char *);
		void Prompt();
		void Led(byte);
		void Led(int, int);
};
#endif

