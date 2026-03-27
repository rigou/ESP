/* rgUI::h
** 12-02-2012
*/

#include "rgUI.h"

rgUI::rgUI() {
	Init_bool = false;
}

/*
** Public interface
*/

void rgUI::Title(char *title_str, char *subtitle_str) {
	if (! Init_bool) {
		Serial.begin(9600);
		pinMode(13, OUTPUT);
		Led(2000, 250);
		Init_bool = true;
	}
	int box_width_int=strlen(title_str) + 4;
	Serial.println("");
	print_line(box_width_int);
	print_box_text(title_str, box_width_int);
	print_box_text(subtitle_str, box_width_int);
	print_line(box_width_int);
}

void rgUI::Prompt() {
	Serial.print("> ");
}

// turn internal led on/off
// state_byt: HIGH/LOW or true/false
void rgUI::Led(byte state_byt) {
	digitalWrite(13, state_byt);
}

// blink internal led during time_int ms
void rgUI::Led(int time_int, int step_int) {
	boolean ledon_bool=false;
	while (time_int > 0) {
		ledon_bool=!ledon_bool;
		Led(ledon_bool);
		delay(step_int);
		time_int-=step_int;
	}
	Led(LOW);
}

/*
** Private implementation
*/

void rgUI::print_line(int width_int) {
	char buffer_str[width_int+1];
	memset(buffer_str, '-', width_int);
	*buffer_str='+';
	*(buffer_str + width_int - 1)='+';
	*(buffer_str + width_int)='\0';
	Serial.println(buffer_str);
}

void rgUI::print_box_text(char *text_str, int width_int) {
	char buffer_str[width_int+1];
	int length_int=strlen(text_str) ;
	if (length_int > width_int - 4)
		length_int=width_int - 4;
	memset(buffer_str, ' ', width_int);
	*buffer_str='+';
	memcpy(buffer_str + 2, text_str, length_int);
	*(buffer_str + width_int - 1)='+';
	*(buffer_str + width_int)='\0';
	Serial.println(buffer_str);
}
