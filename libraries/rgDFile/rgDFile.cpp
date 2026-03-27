/* rgDFile.cpp - Direct access file on SD card
** 2013-04-09
** 2023-11-22 ported to ESP32
*/

/* Copyright (C) 2013-2023 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include <SD.h>
#include "rgDFile.h"

// return the 0-based offset of given record 0-based index
#define DFOffset(idx) ((unsigned long)(idx) * RecordLength)

rgDFile::rgDFile(char *path_str) {
	Path = path_str;
	Nrecords = 0;
	RecordLength = 0;
	State_byt = DFSTATE_CLOSED;
}

// Open a file for direct access on the SD card
// If the file is opened for writing, it will be created if it doesn't already exist (but the directory containing it must already exist)
// Note #1: path_str must specify an absolute path, ie must start with '/'
// Note #2: only one file can be open at a time
// mode_byt: DFSTATE_READ, DFSTATE_WRITE
// Return value: 0=OK, DF_EOF, DF_ERROR, DF_IOERROR
byte rgDFile::Open(char *path_str, const word nrec_int, const word reclen_int, const byte mode_byt) {
	byte retval_byt = 0;
	
	if (State_byt == DFSTATE_CLOSED) {
		Path = path_str;
		Nrecords = nrec_int;
		RecordLength = reclen_int;
		
		boolean exist_bool = SD.exists(path_str);
		File_obj = SD.open(path_str, mode_byt == DFSTATE_WRITE ? FILE_WRITE:FILE_READ);
		if (File_obj) {
			State_byt = mode_byt;
			if (mode_byt == DFSTATE_WRITE && !exist_bool) {
				// initialize the file
				const char *buff_str = "^";
				for (word recidx_int = 0; recidx_int < Nrecords; recidx_int ++)
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

void rgDFile::Close() {
	if (State_byt != DFSTATE_CLOSED) {
		File_obj.close();
		State_byt = DFSTATE_CLOSED;
	}
}

// Delete given file
// Return value: 0 if the removal of the file succeeded, 1 if failed because file is opened or 2=I/O error
// if the file didn't exist, the return value is 0
// Return value: 0=OK, DF_EOF, DF_ERROR, DF_IOERROR
byte rgDFile::Delete() {
	byte retval_byt = 0;
	
	if (State_byt == DFSTATE_CLOSED) {
		if (SD.exists(Path)) {
			if (! SD.remove(Path))
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
byte rgDFile::Read(const word index_int, char *out_buffer_str, word nbytes_int=0) {
	byte retval_byt = 0;
	
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
				for (word idx_int = 0; idx_int < nbytes_int; idx_int ++)
					*(out_buffer_str + idx_int) = '\0';
				if (File_obj.seek(DFOffset(index_int))) {
					if ((word)File_obj.available() >= nbytes_int) {
						for (word idx_int = 0; idx_int < nbytes_int; idx_int ++) {
							int c_int = File_obj.read();
							if (c_int >= 0)
								*(out_buffer_str + idx_int) = (char)c_int;
							else {
								retval_byt = DF_IOERROR; // I/O error on read()
								break;
							}
						}
					}
					else
						retval_byt = DF_ERROR; // last record truncated
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
byte rgDFile::Write(const word index_int, const char *buffer_str, word nbytes_int=0) {
	byte retval_byt = 0;
	
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
				if (File_obj.seek(DFOffset(index_int))) {
					if (File_obj.write((const byte *)buffer_str, nbytes_int) == nbytes_int) {
						if (nbytes_int < RecordLength) {
							// pad the record
							for (word idx_int = nbytes_int; idx_int < RecordLength-1; idx_int ++) {
								if (File_obj.write('.') != 1) {
									retval_byt = DF_IOERROR; // I/O error on write()
									break;
								}
							}
							if (File_obj.write('\n') != 1)
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

