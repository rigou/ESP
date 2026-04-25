/* arduinotx_lib.h - Library
** 09-06-2013
*/


#ifndef arduinotx_lib_h
#define arduinotx_lib_h
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdarg.h>

void aprintfln(const char *fmt_str, ... ) ;
void aprintf(const char *fmt_str, ... ) ;
int serialWrite(char c, FILE *f) ;
char *getProgmemStrArrayValue(char *out_buffer_str, PGM_P *array_str, int idx_int, size_t buffersize_int) ;
int findProgmemStrArrayIndex(PGM_P *array_str, const char *value_str, int nitems_int = 32767) ;
void printProgmemStrArray(PGM_P *array_str, int nitems_int = 32767) ;

#endif