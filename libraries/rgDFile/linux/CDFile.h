/* CDFile.h
** 2013-04-09
*/

/* Copyright (C) 2013 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef CDFile_h
#define CDFile_h

#include <stdio.h>

#define DFSTATE_CLOSED 0
#define DFSTATE_READ 1
#define DFSTATE_WRITE 2

#define DF_ERROR 1
#define DF_IOERROR 2
#define DF_EOF 3

class CDFile {
	private:
		FILE *File_obj;
		unsigned short State_byt;
	
	public:
		CDFile(char *path_str);
	
		char *Path;
		unsigned int Nrecords;
		unsigned int RecordLength;
	
		unsigned short Exists(char *path_str);
		unsigned short Open(char *path_str, const unsigned int nrec_int, const unsigned int reclen_int, const unsigned short mode_byt);
		void Close();
		unsigned short Delete();
		unsigned short Read(const unsigned int index_int, char *out_buffer_str, unsigned int nbytes_int);
		unsigned short Write(const unsigned int index_int, const char *buffer_str, unsigned int nbytes_int);
};
#endif
