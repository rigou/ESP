/* rgDFile.h
** 2013-04-09
** 2023-11-22 ported to ESP32
*/

/* Copyright (C) 2013 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef DFile_h
#define DFile_h

#define DFSTATE_CLOSED 0
#define DFSTATE_READ 1
#define DFSTATE_WRITE 2

#define DF_ERROR 1
#define DF_IOERROR 2
#define DF_EOF 3

#include <SD.h>

#define DFLIB_NAME	"rgDFile" // spaces not permitted
#define DFLIB_VERSION	"v1.0.1"

class rgDFile {
	private:
		File File_obj;
		byte State_byt;
	
	public:
		rgDFile(char *path_str);
	
		char *Path;
		word Nrecords;
		word RecordLength;
	
		byte Open(char *path_str, const word nrec_int, const word reclen_int, const byte mode_byt);
		void Close();
		byte Delete();
		byte Read(const word index_int, char *out_buffer_str, word nbytes_int);
		byte Write(const word index_int, const char *buffer_str, word nbytes_int);
};
#endif
