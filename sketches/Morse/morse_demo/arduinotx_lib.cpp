/* arduinotx_lib.c - Library
** 09-06-2013
  
Copyright (C) 2013 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include "arduinotx_lib.h"

/*
** printf() to Serial --------------------------------------------------------
**
** Before using these functions, you must do the following in setup() to redirect stdout to Serial:
**	stdout = stderr = fdevopen(serialWrite, NULL);
*/

// Send given variable list of args over the serial link and appen \r\n
void aprintfln(const char *fmt_str, ... ) {
	va_list args;
	va_start (args, fmt_str );
	vprintf(fmt_str, args);
	va_end (args);
	puts("\r");
}

// Send given variable list of args over the serial link
void aprintf(const char *fmt_str, ... ) {
	va_list args;
	va_start (args, fmt_str );
	vprintf(fmt_str, args);
	va_end (args);
}

int serialWrite(char c, FILE *f) {
    Serial.write(c);
    return 0;
}

/*
** PROGMEM-related --------------------------------------------------------
*/

// Copy an item of a PROGMEM string array into a RAM buffer
// out_buffer_str : the target buffer, user-allocated
// array_str : the PROGMEM string array
// idx_int : index of the item of array_str to copy into out_buffer_str
// buffersize_int : size of out_buffer_str, at most buffersize_int-1 chars will be copied and a \0 will be appended
char *getProgmemStrArrayValue(char *out_buffer_str, PGM_P *array_str, int idx_int, size_t buffersize_int) {
	char *retval_str = out_buffer_str;
	PGM_P item_ptr = (PGM_P)pgm_read_word(array_str + idx_int);
	if (item_ptr) {
		strncpy_P(out_buffer_str, item_ptr, buffersize_int);
		out_buffer_str[buffersize_int-1] = '\0';
	}
	else {
		*out_buffer_str = '\0';
		retval_str = NULL;
	}
	return retval_str;
}

// Return the index of given item in the given PROGMEM string array or -1 if item not found
// array_str : the PROGMEM string array
// item_str : the string value to search for in array_str
// nitems_int : optional, number of items in array_str;
// if nitems_int is not specified then the last item of the array must be a NULL pointer else this function will probably crash your program
int findProgmemStrArrayIndex(PGM_P *array_str, const char *value_str, int nitems_int) {
	int retval_int = -1;
	int idx_int = 0;
	PGM_P item_ptr = NULL;
	do {
		item_ptr = (PGM_P)pgm_read_word(array_str + idx_int);
		if (item_ptr) {
			if (strcmp_P(value_str, item_ptr) == 0)
				retval_int = idx_int;
			else
				idx_int++;
		}
	} while (idx_int < nitems_int && item_ptr && retval_int == -1);
	return retval_int;
}

// print a PROGMEM string array
// array_str : the PROGMEM string array
// nitems_int : optional, maximum number of items to print;
// if nitems_int is not specified then the last item of the array must be a NULL pointer else this function will probably crash your program
void printProgmemStrArray(PGM_P *array_str, int nitems_int) {
	char line_str[10];
	int idx_int = 0;
	char *value_str = NULL;
	do {
		value_str = getProgmemStrArrayValue(line_str, array_str, idx_int, 10);
		if (value_str) {
			aprintf("#%d: \"%s\"\n", idx_int, value_str);
			idx_int++;
		}
	} while (value_str && idx_int < nitems_int);
}

