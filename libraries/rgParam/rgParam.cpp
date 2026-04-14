/* This program is published under the GNU General Public License. 
 * This program is free software and you can redistribute it and/or modify it under the terms
 * of the GNU General  Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.
 * See the GNU General Public License for more details : https://www.gnu.org/licenses/ *GPL 
 *
 * Installation, usage : https://github.com/rigou/nRF24L01-FHSS/
 *
 * Settings persistence on ESP32 and ESP8266
   Do not create several instances of this class : only a single instance of this object is possible
     due to LittleFS library limitation, but you can reuse the same object to open different files
     one after each other (see test program FParam.ino)
   The LittleFS filesystem requires a SPIFFS partition : in the Arduino IDE, select any SPIFFS scheme
     in Tools / Partition scheme
*/
#include "rgParam.h"

/* call this method to allocate resources for this object
	path_str absolute path or simple file name, or NULL ; if not NULL then copy it into mPath_str
	format_fs_if_failed_int tells how to process mount errors : 
		1=try to format the fs and then retry to mount it (default),
		0=return error
	max_len_path is the maximum length of a filename like "settings.txt" (possibly prefixed by a folder name like "custom/settings.txt"), 
		not counting a leading '/' (allowed but not required), nor the trailing '\0'
	if strlen(path_str) > max_len_path then it will be silently truncated
   Return value: 0=success, 1,2,3 error code : see below
*/
int rgParam::Init(const char *path_str, 
	int format_fs_if_failed_int, 
	int max_len_path, 
	int max_len_key, 
	int max_len_value, 
	int max_records
	) {
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(%s, %d, %d, %d, %d, %d)", __func__, path_str?path_str:"NULL", format_fs_if_failed_int, max_len_path, max_len_key, max_len_value, max_records);
	Trace();
#endif
	int retval_int=0;

	MaxLenPath=max_len_path;
	MaxLenKey=max_len_key;
	MaxLenValue=max_len_value;
	MaxRecords=max_records;

	mBuffer_str=(char *)calloc(RGPARAM_BUFFER_LEN, sizeof(char));
	// MaxLenPath+2 chars because +1 for the trailing '\0' and +1 for the leading '/' which set_current_path() will add if not already there
	mPath_str=(char *)calloc(MaxLenPath+2, sizeof(char)); 

	mKeys_str=(char **)calloc(MaxRecords, sizeof(char *));
	for (int idx=0; idx<MaxRecords; idx++)
		mKeys_str[idx]=(char *)calloc(MaxLenKey+1, sizeof(char));
	
	mValues_str=(char **)calloc(MaxRecords, sizeof(char *));
	for (int idx=0; idx<MaxRecords; idx++)
		mValues_str[idx]=(char *)calloc(MaxLenValue+1, sizeof(char));

	set_current_path(path_str);
	
	// do not format partition automatically if begin() fails
	if (LittleFS.begin(false))
		retval_int=0; // success, will also return success if called multiple times
	else {
		// mount failed
		if (format_fs_if_failed_int) {
			if (LittleFS.format()) {
				// retry mount
				if (LittleFS.begin(false))
					retval_int=0; // success
				else
					retval_int=3; // remount failed
			}
			else
				retval_int=2; // format failed
		}
		else
			retval_int=1; // mount failed
	}

#if RGPARAM_TRACE >= 1
 	snprintf(Trace_str, TRACELEN, "%s(%s, %d) returns %d", __func__, path_str?path_str:"NULL", format_fs_if_failed_int, retval_int);
	Trace();
#endif
	return retval_int;
}

/* call this method to close the file, unmount the LittleFS file system and free memory allocated by Init()
   this is required if you want to reuse the same object with a different file
*/
void rgParam::End(void) {
	LittleFS.end();
	free(mBuffer_str);
	free(mPath_str);
	for (int idx=0; idx<MaxRecords; idx++)
		free(mKeys_str[idx]);
	free(mKeys_str);
	for (int idx=0; idx<MaxRecords; idx++)
		free(mValues_str[idx]);
	free(mValues_str);
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s() complete", __func__);
	Trace();
#endif
}

/*  Load the whole file in RAM
	path_str optional, absolute path or simple file name, or NULL ; if not NULL then copy it into mPath_str
	if path_str == NULL then we use mPath_str, else we copy path_str into mPath_str
    Return value : number of key/value pairs found, or a negative value on error
	-1	file not found
	-2	key empty or too long
	-3	key/value separator not found
	-4	value too long
	-5	too many lines
	-6	line too long or missing newline
*/
int rgParam::Load(const char *path_str) {
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(%s)", __func__, path_str?path_str:mPath_str);
	Trace();
#endif
    int retval_int=0;
    Clear();

	set_current_path(path_str);
    File file_obj=LittleFS.open(mPath_str,"r");
    int line_int=0;
    if (file_obj) {
		int eof_int=0;
		while (!eof_int && !retval_int) {
			switch (read_line(&file_obj, RGPARAM_BUFFER_LEN, mBuffer_str)) {
				case -2:
					eof_int=1;
					break; // EOF

				case -1:
					retval_int=-6; // line too long or missing '\n'
					break;

				default:
					if (line_int==MaxRecords)
						retval_int=-5; // too many lines
					else {
						char *key_str=NULL;
						char *value_str=NULL;
						if (parse_line(mBuffer_str, &key_str, &value_str)==0) {
							if (strlen(key_str) && strlen(key_str) <= MaxLenKey) {
								strncpy(mKeys_str[line_int], key_str, MaxLenKey);
								if (strlen(value_str) <= MaxLenValue) {
									strncpy(mValues_str[line_int], value_str, MaxLenValue);
									line_int++;
#if RGPARAM_TRACE >= 1
									snprintf(Trace_str, TRACELEN, "%s(%s) reads line %d: key \"%s\" value \"%s\"", __func__, path_str?path_str:mPath_str, line_int, key_str, value_str);
									Trace();
#endif
								}
								else
									retval_int=-4; // value too long (empty value is ok)
							}
							else
								retval_int=-2; // key empty or too long
						}
						else
							retval_int=-3; // mSeparator not found
					}
                	break;
			}
		}
        file_obj.close();
    }
    else
        retval_int=-1; // file not found

    if (retval_int == 0)
        retval_int=line_int;
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(%s) returns %d", __func__, path_str?path_str:mPath_str, retval_int);
	Trace();
#endif
    return retval_int;
}

// path_str optional, absolute path or simple file name, or NULL ; if not NULL then copy it into mPath_str
// if path_str == NULL then use mPath_str else copy path_str into mPath_str
// return value: number of key/value pairs written, negative value on error  (do not modify these negative values)
int rgParam::Save(const char *path_str) {
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(%s)", __func__, path_str?path_str:mPath_str);
	Trace();
#endif
	int retval_int=0;
	int lines_int=0;
        
    set_current_path(path_str);

    // truncate existing file
	LittleFS.remove(mPath_str);
    File file_obj=LittleFS.open(mPath_str,"w");
    if (file_obj) {
        char *buff_str=mBuffer_str;
        int bufflen_int=0;
        int idx_int=0;
		while (idx_int < MaxRecords) {
            char *key_str=mKeys_str[idx_int];
			char *value_str=mValues_str[idx_int];
            int keylen_int=strlen(key_str);
            if (keylen_int>0) { // else ignore empty key
            	memset(buff_str, '\0', RGPARAM_BUFFER_LEN);
                strncpy(buff_str, key_str, MaxLenKey);
                buff_str[keylen_int]=mSeparator;
                strcat(buff_str, value_str);
                bufflen_int=strlen(buff_str);
                buff_str[bufflen_int]='\n';
                buff_str[++bufflen_int]='\0';
                int count_int=file_obj.write((const uint8_t *)buff_str, bufflen_int);
				if (count_int==bufflen_int) {
					++lines_int;
#if RGPARAM_TRACE >= 1
					snprintf(Trace_str, TRACELEN, "%s(%s) writes line %d key \"%s\" value \"%s\"", __func__, path_str?path_str:mPath_str, lines_int, key_str, value_str);
					Trace();
#endif
				}
				else
					retval_int=-2; // i/o error writing data
            }
            idx_int++;
        }
        file_obj.close();
    }
    else
        retval_int=-1; // i/o error creating file

    if (retval_int == 0)
        retval_int=lines_int;
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(%s) returns %d", __func__, path_str?path_str:mPath_str, retval_int);
	Trace();
#endif
    return retval_int;
}

// Deletes the file given its absolute path
// path_str optional, absolute path or simple file name, or NULL ; if not NULL then copy it into mPath_str
// if path_str == NULL then use mPath_str else copy path_str into mPath_str
// Return value: 0=file was deleted successfully, 1=error 
int rgParam::Remove(const char *path_str) {
	int retval_int=0;
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(%s)", __func__, path_str?path_str:mPath_str);
	Trace();
#endif
	set_current_path(path_str);
	if (!LittleFS.remove(mPath_str))
		retval_int=1;

	return retval_int;
}
 
// clears all keys and values in memory arrays only
void rgParam::Clear(void) {
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(void)", __func__);
	Trace();
#endif
    int idx_int=0;
    while (idx_int < MaxRecords) {
        *mKeys_str[idx_int]='\0';
        *mValues_str[idx_int]='\0';
        idx_int++;
    }
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s() returns void", __func__);
	Trace();
#endif
}

// stores value of given key into given user-allocated value_out_str
// set key_str=NULL to retrieve the index of an empty keypair
// set value_out_str=NULL to retrieve the index of the key only
// return value: 0-based index of the key in mKeys_str[][] or -1 not found
int rgParam::GetKeyStr(const char *key_str, char *value_out_str) {
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s)", __func__, key_str?key_str:"NULL");
	Trace();
#endif
    int idx_int=MaxRecords-1;
	if (value_out_str)
		*value_out_str='\0';
    while (idx_int >=0) {
        if (key_str) {
            if (strncmp(mKeys_str[idx_int], key_str, MaxLenKey)==0) {
                if (value_out_str)
                    strncpy(value_out_str, mValues_str[idx_int], MaxLenValue);
                break;
            }
        }
        else {
            if (*mKeys_str[idx_int]=='\0')
                break;
        }
        idx_int--;
    }
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s) returns \"%s\" (%d)", __func__, key_str?key_str:"NULL", value_out_str?value_out_str:"NULL", idx_int);
	Trace();
#endif
    return idx_int;
}

// copy given key and given value in memory arrays
// return value: 0-based index of the key in mKeys_str[][] or -1 if cannot add a key (mKeys_str array full)
int rgParam::SetKeyStr(const char *key_str, const char *value_str) {
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s) \"%s\"", __func__, key_str?key_str:"NULL", value_str?value_str:"NULL");
	Trace();
#endif
    int idx_int=GetKeyStr(key_str, NULL);
    if (idx_int<0) {
        idx_int=GetKeyStr(NULL, NULL);
        if (idx_int>=0)
            strncpy(mKeys_str[idx_int], key_str, MaxLenKey);
    }
    if (idx_int>=0)
        strncpy(mValues_str[idx_int], value_str, MaxLenValue);
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s, \"%s\") returns %d", __func__, key_str?key_str:"NULL", value_str?value_str:"NULL", idx_int);
	Trace();
#endif
    return idx_int;
}

// stores the unsigned integer value of given key into given *value_out_int
// the value is expected to be recorded as an hexadecimal string
// return value: 0-based index of the key in mKeys_str[][] or -1 not found or -2 invalid value
int rgParam::GetKeyInt(const char *key_str, unsigned int *value_out_int) {
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s)", __func__, key_str?key_str:"NULL");
	Trace();
#endif
	*value_out_int=0;
	char value_str[MaxLenValue+1];
	int retval_int=GetKeyStr(key_str, value_str);
	if (retval_int>=0) {
		if (sscanf(value_str, "%x", value_out_int)!=1)
			retval_int=-2; // invalid value
	}
	else {
		retval_int=-1; // key not found
		*value_out_int=UINT32_MAXVALUE;
	}
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s) returns %u (%x)", __func__, key_str?key_str:"NULL", retval_int, *value_out_int);
	Trace();
#endif
	return retval_int;
}

// record the given value of given key ; value is written as an hexadecimal string
// return value: 0-based index of the key in mKeys_str[][] or -1 if cannot add a key (mKeys_str array full)
int rgParam::SetKeyInt(const char *key_str, unsigned int value_int) {
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s) %u", __func__, key_str?key_str:"NULL", value_int);
	Trace();
#endif
	int retval_int=0;
	char value_str[MaxLenValue+1];
	snprintf(value_str, MaxLenValue+1, "%x", value_int);
	retval_int=SetKeyStr(key_str, value_str);
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s, %u) returns %d", __func__, key_str?key_str:"NULL", value_int, retval_int);
	Trace();
#endif	
	return retval_int;
}

// clears given key and its corresponding value in memory arrayss
// return value: 0-based index of the key in mKeys_str[][] or -1 not found
int rgParam::DelKey(const char *key_str) {
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s)", __func__, key_str?key_str:"NULL");
	Trace();
#endif
    int idx_int=GetKeyStr(key_str, NULL);
	if (idx_int>=0) {
		*mKeys_str[idx_int]='\0';
		*mValues_str[idx_int]='\0';
	}
#if RGPARAM_TRACE >= 2
	snprintf(Trace_str, TRACELEN, "%s(%s) returns %d", __func__, key_str?key_str:"NULL", idx_int);
	Trace();
#endif
	return idx_int;
}

// copy the raw contents of the file into given user-allocated buffer
// all characters are copied except '\r'
// path_str optional, absolute path or simple file name, or NULL ; if not NULL then copy it into mPath_str
// return value: number of characters read from file and stored in buffer, -1 on error (buffer too small is not an error)
int rgParam::Dump(const char *path_str, int buffer_size_int, char *buffer_out_str) {
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(%s, %d)", __func__, path_str?path_str:"NULL", buffer_size_int);
	Trace();
#endif
	int retval_int=0;
	int idx_int=0;

	set_current_path(path_str);

	File file_obj=LittleFS.open(mPath_str,"r");
	if (file_obj) {
		while (file_obj.available() && idx_int < buffer_size_int-1) {
			char this_char=file_obj.read();
			if (this_char!='\r')
				buffer_out_str[idx_int++]=this_char;
		}
		buffer_out_str[idx_int]='\0';
		retval_int=idx_int;
		file_obj.close();
	}
	else
		retval_int=-1; // file not found
	
#if RGPARAM_TRACE >= 1
	snprintf(Trace_str, TRACELEN, "%s(%s, %d) returns %d", __func__, path_str?path_str:"NULL", buffer_size_int, retval_int);
	Trace();
#endif
	return retval_int;
}

// prints the formatted contents of the file on Serial
// path_str optional, absolute path or simple file name, or NULL ; if not NULL then copy it into mPath_str
void rgParam::SerialDump(const char *path_str) {
	set_current_path(path_str);
	Serial.printf("file %s\n", mPath_str);
	File file_obj=LittleFS.open(mPath_str,"r");
	if (file_obj) {
		while (1) {
			if (read_line(&file_obj, RGPARAM_BUFFER_LEN, mBuffer_str) == -2)
				break; // EOF
			else {
				char *key_str=NULL;
				char *value_str=NULL;
				if (parse_line(mBuffer_str, &key_str, &value_str)==0)
					Serial.printf("key=\"%s\", value=\"%s\"\n", key_str, value_str);
				else
					Serial.printf("invalid=\"%s\"\n", mBuffer_str);
			}
		}
		file_obj.close();
	}
    else
		Serial.printf("file not found %s\n", mPath_str);
}

// Return pointers to the key and to the value from raw line in given buffer
// key and value strings are both terminated by \0
// the contents of given buffer is changed (mSeparator replaced by \0)
// Return value: 0=Ok, -1=mSeparator not found
int rgParam::parse_line(char *line_str_out, char **key_ptr_out, char **value_ptr_out) {
	int retval_int=0;
	char *tab_ptr=strchr(line_str_out, mSeparator);
	if (tab_ptr) {
		*tab_ptr='\0';
		*key_ptr_out=line_str_out;
		*value_ptr_out=tab_ptr+1;
	}
	else {
		*value_ptr_out=NULL;
		retval_int=-1;
	}
	return retval_int;
}

// Read a line in given file and store it into given buffer
// buffer must have room to store the terminating '\0' char
// buffer_size_int is the total length, including the terminating '\0'
// any '\r' is ignored, '\n' is not copied to the buffer
// Return values:
// * length of the line in given buffer (may be 0) if the line was read correctly
// * or -1 and strlen(buffer)>0 if the line was too long to fit in the buffer or if we read
//     the last line of the file and it was not terminated by '\n'
//     whatever data was read is available in the buffer
// * or -2 and strlen(buffer)=0 on EOF 
int rgParam::read_line(File *file_obj, int buffer_size_int, char *buffer_out_str) {
	int retval_int=-1;
	int char_counter_int=0;
	char *buffer_ptr=buffer_out_str;
	memset(buffer_out_str, '\0', buffer_size_int);
	while (file_obj->available()) {
		if (char_counter_int < buffer_size_int-1) {
			char this_char=file_obj->read();
			if (this_char!='\r') {
				if (this_char=='\n') {
					retval_int=char_counter_int;
					break;
				}
				else {
					*buffer_ptr++=this_char;
					char_counter_int++;
				}
			}
		}
		else {
			retval_int=-1; // '\n' not found before end of buffer
			break;
		}
	}
	if (retval_int==-1 && char_counter_int==0)
		retval_int=-2; // EOF
	return retval_int;
}

// format given path into global buffer mPath_str, with a leading '/' because LittleFS requires absolute path
// path_str is an absolute path or a simple file name, or NULL
// if strlen(path_str) >= MaxLenPath+2 then the value copied in mPath_str will be truncated
void rgParam::set_current_path(const char *path_str) {
	if (path_str) {
		mPath_str[0]='/';
		byte shift=(path_str[0]=='/')?0:1;
		strncpy(mPath_str+shift, path_str, MaxLenPath+2-shift);
		mPath_str[MaxLenPath+1]='\0'; // because if strlen(path_str) >= MaxLenPath+2 then strncpy does not append a null terminator (\0) to mPath_str
	}
}

// call this method to change the field separator
void rgParam::SetSeparator(char sep_chr) {
	mSeparator=sep_chr;
}

#if RGPARAM_TRACE >= 1
void rgParam::Trace(const char *text_str) {
	if (!text_str)
		text_str=Trace_str;
	Serial.printf("  rgParam.%s\n", text_str);
}
#endif

