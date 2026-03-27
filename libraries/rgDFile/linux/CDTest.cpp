/* CDTest.cpp - Direct file read/write test
**
** g++ -o CDTest CDTest.cpp CDFile.cpp && ls -l CDTest
**
** 2013-04-09
** 2023-11-09 changed value of DFPATH, compiled and tested ok on Debian 11.8
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CDFile.h"

char DFPATH[] = "/tmp/CDTEST.TMP";
CDFile DFile_obj(DFPATH);

int main(int argc, char** argv) {
	const unsigned int NRECORDS = 10000;
	const unsigned int RECSIZE = 20;
	unsigned long open_errcount_lng = 0;
	unsigned long write_errcount_lng = 0;
	unsigned long read_errcount_lng = 0;

	DFile_obj.Delete();

	char record_str[RECSIZE + 1];

	// Write
	unsigned short retval_byt = DFile_obj.Open(DFPATH, NRECORDS, RECSIZE, DFSTATE_WRITE);
	if (retval_byt == 0) {
		for (unsigned int reccount_int = 0; reccount_int < NRECORDS; reccount_int++)  {
			sprintf(record_str, "%d", reccount_int);
			if (reccount_int % (NRECORDS/10) == 0)
				printf("Write record %s\n", record_str);
			retval_byt = DFile_obj.Write(reccount_int, record_str, strlen(record_str));
			if (retval_byt != 0) {
				printf("Write() error %d\n", retval_byt);
				write_errcount_lng++;
			}
		}
		DFile_obj.Close();
	}
	else {
		printf("Open(DFSTATE_WRITE) error %d\n",retval_byt);
		open_errcount_lng++;
	}
	
	// Read
	retval_byt = DFile_obj.Open(DFPATH, NRECORDS, RECSIZE, DFSTATE_READ);
	if (retval_byt == 0) {
		for (unsigned int reccount_int = 0; reccount_int < NRECORDS; reccount_int++)  {
			if (reccount_int % (NRECORDS/10) == 0)
				printf("Read record %u\n", reccount_int);
			retval_byt = DFile_obj.Read(reccount_int, record_str, RECSIZE);
			record_str[RECSIZE] = '\0';
			if (retval_byt == 0) {
				if ((unsigned int)atoi(record_str) != reccount_int) {
					printf("Invalid record %u\n", reccount_int);
					read_errcount_lng++;
				}
			}
			else {
				printf("Read() error %d\n", retval_byt);
				read_errcount_lng++;
			}
		}
		DFile_obj.Close();
	}
	else {
		printf("Open(DFSTATE_READ) error %d\n", retval_byt);
		open_errcount_lng++;
	}

	printf("open errors: %lu, read errors: %lu, write errors: %lu\n", open_errcount_lng, write_errcount_lng, read_errcount_lng);
}

	
