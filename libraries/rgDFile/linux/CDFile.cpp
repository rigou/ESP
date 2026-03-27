/* CDFile.cpp
** 2013-04-09
*/

/* Copyright (C) 2013 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include "CDFile.h"
#include <unistd.h>

// return the 0-based offset of given record 0-based index
#define DFOffset(idx) ((unsigned long)(idx) * RecordLength)

CDFile::CDFile(char *path_str) {
	Path = path_str;
	Nrecords = 0;
	RecordLength = 0;
	State_byt = DFSTATE_CLOSED;
}

// Test if given file exists and is readable
// Return value: 1=OK, 0=file not found or not readable
unsigned short CDFile::Exists(char *path_str) {
	//~ printf("CDFile::Exists(%s)\n", path_str);
	unsigned short retval_byt = 0;
	FILE *f_obj = fopen(path_str, "r");
	retval_byt = (f_obj == NULL ? 0:1);
	if (retval_byt)
		fclose(f_obj);
	return retval_byt;
}
	
// Open a file for direct access
// If the file is opened for writing, it will be created if it doesn't already exist (but the directory containing it must already exist)
// Note: only one file can be open at a time. 
// mode_byt: DFSTATE_READ, DFSTATE_WRITE
// Return value: 0=OK, DF_EOF, DF_ERROR, DF_IOERROR
unsigned short CDFile::Open(char *path_str, const unsigned int nrec_int, const unsigned int reclen_int, const unsigned short mode_byt) {
	//~ printf("CDFile::Open(%s, %u, %u, %u)\n", path_str, nrec_int, reclen_int, mode_byt);
	unsigned short retval_byt = 0;
	
	if (State_byt == DFSTATE_CLOSED) {
		Path = path_str;
		Nrecords = nrec_int;
		RecordLength = reclen_int;
		unsigned short exist_byt = Exists(path_str);
		
		File_obj = fopen(path_str, mode_byt == DFSTATE_WRITE ? "w":"r");
		if (File_obj) {
			State_byt = mode_byt;
			if (mode_byt == DFSTATE_WRITE && !exist_byt) {
				// initialize the file
				const char *buff_str = "^";
				for (unsigned int recidx_int = 0; recidx_int < Nrecords; recidx_int ++)
					Write(recidx_int, buff_str, 1);
			}
		}
		else
			retval_byt = DF_IOERROR;
	}
	else
		retval_byt = DF_ERROR; // must close first

	return retval_byt;
}

void CDFile::Close() {
	//~ printf("CDFile::Close()\n");
	if (State_byt != DFSTATE_CLOSED) {
		fclose(File_obj);
		State_byt = DFSTATE_CLOSED;
	}
}

// Delete given file
// Return value: 0 if the removal of the file succeeded, 1 if failed because file is opened or 2=I/O error
// if the file didn't exist, the return value is 0
// Return value: 0=OK, DF_EOF, DF_ERROR, DF_IOERROR
unsigned short CDFile::Delete() {
	//~ printf("CDFile::Delete()\n");
	unsigned short retval_byt = 0;
	
	if (State_byt == DFSTATE_CLOSED) {
		if (Exists(Path)) {
			if (! unlink(Path))
				retval_byt = DF_IOERROR;
		}
	}
	else
		retval_byt = DF_ERROR; // must close first

	return retval_byt;
}

// Read given record (0-based index) into given buffer
// nbytes_int: number of chars to retrieve from the record; optional, default=record size; you may specify less
// Return value: 0=OK, DF_EOF, DF_ERROR, DF_IOERROR
unsigned short CDFile::Read(const unsigned int index_int, char *out_buffer_str, unsigned int nbytes_int=0) {
	//~ printf("CDFile::Read(%u, buffer, %u)\n", index_int, nbytes_int);
	unsigned short retval_byt = 0;
	
	if (State_byt == DFSTATE_READ) {
		if (index_int < Nrecords) {
			// validate arg nbytes_int
			if (nbytes_int > 0) {
				if (nbytes_int > RecordLength)
					retval_byt = DF_ERROR;  // invalid arg nbytes_int
			}
			else
				nbytes_int = RecordLength; // default
			
			if (retval_byt == 0) {
				for (unsigned int idx_int = 0; idx_int < nbytes_int; idx_int ++)
					*(out_buffer_str + idx_int) = '\0';
				if (fseek(File_obj, DFOffset(index_int), SEEK_SET) == 0) {
					for (unsigned int idx_int = 0; idx_int < nbytes_int; idx_int ++) {
						int c_int = fgetc(File_obj);
						if (c_int != EOF)
							*(out_buffer_str + idx_int) = (char)c_int;
						else {
							retval_byt = DF_IOERROR; // I/O error on read() : last record truncated
							break;
						}
					}
				}
				else
					retval_byt = DF_IOERROR; // I/O error on seek()
			}
		}
		else
			retval_byt = DF_EOF;  // index out of range
	}
	else
		retval_byt = DF_ERROR;  // invalid open mode

	return retval_byt;
}

// Write given record (0-based index) using data from given buffer
// nbytes_int: number of chars to write from the buffer; optional, default=record size
// you may specify less but the record will be padded anyway to its full length
// Return value: 0=OK, DF_EOF, DF_ERROR, DF_IOERROR
unsigned short CDFile::Write(const unsigned int index_int, const char *buffer_str, unsigned int nbytes_int=0) {
	//~ printf("CDFile::Write(%u, %s, %u)\n", index_int, buffer_str, nbytes_int);
	unsigned short retval_byt = 0;
	
	if (State_byt == DFSTATE_WRITE) {
		if (index_int < Nrecords) {
			// validate arg nbytes_int
			if (nbytes_int > 0) {
				if (nbytes_int > RecordLength)
					retval_byt = DF_ERROR;  // invalid arg nbytes_int
			}
			else
				nbytes_int = RecordLength; // default
			
			if (retval_byt == 0) {
				if (fseek(File_obj, DFOffset(index_int), SEEK_SET) == 0) {
					if (fwrite (buffer_str, 1, nbytes_int, File_obj) == nbytes_int) {
						if (nbytes_int < RecordLength) {
							// pad the record
							for (unsigned int idx_int = nbytes_int; idx_int < RecordLength-1; idx_int ++) {
								if (fputc('.', File_obj) != '.') {
									retval_byt = DF_IOERROR; // I/O error on write()
									break;
								}
							}
							if (fputc('\n', File_obj) != '\n')
								retval_byt = DF_IOERROR; // I/O error on write()
						}
					}
					else
						retval_byt = DF_IOERROR; // I/O error on write()
				}
				else
					retval_byt = DF_IOERROR; // I/O error on seek()
			}
		}
		else
			retval_byt = DF_EOF;  // index out of range
	}
	else
		retval_byt = DF_ERROR;  // invalid open mode
	return retval_byt;
}

