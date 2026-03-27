/* UserCmd.cpp - Example of a user-defined command interpreter

Using Minicom with ESP32 boards :
Warning: the Arduino Serial Monitor is not suitable for XModem files transfers over the built-in USB port
If you intend to transfer files between the ESP32 board and your computer,
you must use a serial terminal like minicom with : minicom -b 115200 -D /dev/ttyUSB0

Configure Minicom :
	ESP32 boards do not implement serial hardware flow control
	You must disable this feature in Minicom, otherwise Serial.available() will not detect incoming characters :
		^A O / "Serial port setup" / Hardware Flow Control = no
	And set end of line termination as CR+LF
		^A U "Add carriage return ON"
		you can also set it in the default settings: 
		^A O "Screen and keyboard" / "Add carriage return"
	Persist Minicom configuration :    
		^A O "Save setup as dfl"

	Other Minicom key bindings :
		type Ctrl-A N    to prefix each line with a timestamp
		type Ctrl-A Z    to display minicom menu
		type Ctrl-A X    to quit
*/

#include <LittleFS.h>
#include <rgStr.h>
#include <XModem.h> // by Thomas Lowry (aka gilman88) v1.0.3 origin https://docs.arduino.cc/libraries/xmodem/

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

#include "UserCmd.h"

#ifdef DB_UART1_ENABLED
#ifdef XMODEM_USE_UART1
#error Cannot use UART1 for debug and xmodem simultaneously
#endif
#endif

#ifdef XMODEM_USE_UART1
#include <HardwareSerial.h>
#ifndef DB_UART1_ENABLED // else Serial1 already declared by rgDebug.h
	// The RX and TX pins of UART1 are assigned to GPIO10 and GPIO9 by default
	// but they are are connected to the flash memory in the ESP32 board.
	// Hence, we will have to reassign the pins for UART1 for serial communication. 
	// The ESP32 board is capable to use almost all GPIO pins for serial connections,
	HardwareSerial SerialPort1(1); // declare this before #include <rgDebug.h>
#endif
#endif

static File TransFile_obj; // used by import/export file

// initialize the command interpreter
// the standard serial port is already opened
void UserCmd::Setup() {
#ifdef DB_UART1_ENABLED
	Serial.println("To watch the output of UART1, attach a serial terminal with: minicom -b 9600 -D /dev/ttyUSB1");
	SerialPort1.begin(9600, SERIAL_8N1, UART1_RX_GPIO, UART1_TX_GPIO);
#endif
	trprintf("*** %s %s() : %s %s\n", __FILE_NAME__, __FUNCTION__, CMDLIB_NAME, CMDLIB_VERSION);
#ifdef XMODEM_USE_UART1
	dbprintf("%s using UART1 (RX GPIO %d, TX GPIO %d) for import/export\n", __FILE_NAME__, UART1_RX_GPIO, UART1_TX_GPIO);
#endif

	// do not format partition if begin() fails
	if (!LittleFS.begin(false)) {
		Serial.println("LittleFS filesystem not found");
		Serial.println("format flash storage with command 'F'");
	}
	FilesCount=list_root_dir(false, FileNames);
	trprintf("*** %s %s() returns, %d file(s) found\n", __FILE_NAME__, __FUNCTION__, FilesCount);
}

// return value: number of characters received before CR or LF, 0 if received ESC, -1=out of memory
int UserCmd::Input(size_t max_input_length) {
	trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
	int input_length=0;
	char *line_buffer=(char *)calloc(max_input_length+1, sizeof(char));
	if (line_buffer) {
		input_length=serial_input(line_buffer, max_input_length);
		if (input_length>0) {
			Serial.print('\n');
			if (input_length!=USER_CANCEL) {
				TrimCommentAndWhitespace(line_buffer);
				if (strlen(line_buffer)) {
					char arg1='\0';
					char arg2[MAXNAMELEN];
					char *arg3=(char *)calloc(max_input_length+1, sizeof(char));
					if (arg3) {
						parse_command_line(line_buffer, &arg1, arg2, arg3, max_input_length);
						exec_command(arg1, arg2, arg3, max_input_length);
						free(arg3);
					}
					else {
						Serial.println("out of memory");
						input_length=-1;
					}
				}
			}
			else
				input_length=0;
		}
		free(line_buffer);
	}
	else {
		Serial.println("out of memory");
		input_length=-1;
	}
	trprintf("*** %s %s() returns %d\n", __FILE_NAME__, __FUNCTION__, input_length);
	serial_prompt();
	return input_length;
}

// arg1_out required, command (length = 1 char)
// arg2_out optional, file number or file name (MAXNAMELEN), 
//		arg2_out will be a zero-length string if not found in command_str
// arg3_out optional, for command 'a' only : double-quoted text (max_input_length) up to the end of the input,
//		 arg3_out will be a zero-length string if not found in command_str
void UserCmd::parse_command_line(char *command_str, char *arg1_out, char *arg2_out, char *arg3_out, size_t max_input_length) {
	trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
	*arg2_out='\0';
	*arg3_out='\0';
	sscanf(command_str, "%c %s", arg1_out, arg2_out);
	// parse the text for command 'a'
	if (*arg1_out=='a') {
		char *quote=strchr(command_str, '"');
		if (quote) {
			strncpy(arg3_out, quote+1, max_input_length);
			// delete the trailing quote and newline(s), if any
			int textlen=strlen(arg3_out);
			while (textlen && (arg3_out[textlen-1]=='\n' || arg3_out[textlen-1]=='\r' || arg3_out[textlen-1]=='"')) {
				arg3_out[textlen-1]='\0';
				--textlen;
			}
		}
	}
	trprintf("*** %s %s() returns\n", __FILE_NAME__, __FUNCTION__);
}

void UserCmd::PrintHelp(void) {
	// interesting C++11 syntax of a raw string literal : R"( multiline_text )"
	Serial.println(
R"(>>> COMMAND <<<
	F format flash storage
	h this help
	l list files
	i file_name    import text file
	e file_number  export text file
	c file_name    create file
	D file_number  delete file
	p file_number  print file
	a file_number "text" append double-quoted text to file)"
	);
}

// arg1 required, command (length = 1 char)
// arg2 optional, file number or file name (MAXNAMELEN), 
//		arg2 is a zero-length string if not defined
// arg3 optional, for command 'a' only : double-quoted text (max_input_length) up to the end of the input,
//		 arg3 is a zero-length string if not defined
void UserCmd::exec_command(char arg1, const char *arg2, const char *arg3, size_t max_input_length) {
	trprintf("*** %s %s(%c, \"%s\") begin\n", __FILE_NAME__, __FUNCTION__, arg1, arg2);
	char path[MAXPATHLEN];
	int file_number=0;
	if (strlen(arg2))
		file_number=atoi(arg2); // >=1

	switch (arg1) {
		case 'h':
			PrintHelp();
			break;

		case 'F':
			Serial.println("formatting LittleFS...");
			if (LittleFS.format()) {
				Serial.println("LittleFS format success");
				LittleFS.begin(false); // do not format if begin() fails again
			}
			else
				Serial.println("format error");
			break;

		case 'l':
			Serial.println("--------------------");
			FilesCount=list_root_dir(true, FileNames);
			Serial.println("--------------------");
			break;

		case 'i':
			if (strlen(arg2)>0) {
				*path='/';
				strncpy(path+1, arg2, MAXPATHLEN-1);
#ifdef XMODEM_USE_UART1
				Serial.printf("To import file %s :\n", path+1);
				Serial.println("1) enter \"y\"");
				Serial.println("2) send your file over the secondary serial port with :");
				Serial.printf("PORT=/dev/ttyUSB1 ; stty -F $PORT %lu && lsx -v %s >$PORT <$PORT\n", UserCmd::XMODEM_SERIAL_SPEED, path+1);
#else
				Serial.println("Warning: the Arduino Serial Monitor is not suitable for files transfers over the built-in USB port");
				Serial.println("use a serial terminal like minicom with : minicom -b 115200 -D /dev/ttyUSB0");
				Serial.printf("Procedure to import file %s :\n", path+1);
				Serial.println("1) enter \"y\" and close this terminal when prompted");
				Serial.println("2) send your file with :");
				Serial.printf("PORT=/dev/ttyUSB0 ; stty -F $PORT %lu && lsx -v %s >$PORT <$PORT\n", UserCmd::XMODEM_SERIAL_SPEED, path+1);
				Serial.println("3) reopen the terminal");
#endif
				Serial.println("");
				char input=enter_one_char("Enter \"y\" to continue, or enter \"n\" to cancel");
				if (input=='y') {
#ifdef XMODEM_USE_UART1
					Serial.println("Send your file now...");
#else
					Serial.println("Close this terminal now (minicom : Ctrl-A X)");
#endif
					int result=xmodem_import_file(path);
#ifndef XMODEM_USE_UART1
					enter_one_char("Press Enter to continue...");
#endif
					Serial.printf("\nTransfer of file %s ", path+1);
					if (result==0) {
						Serial.println("successful");
						exec_command('l', "", "", 0);
					}
					else
						Serial.printf("failed, error %d\n", result);
				}
				else
					Serial.println("Transfer canceled");
			}
			break;

		case 'e':
			if (get_abs_path(file_number, FilesCount, path)==0) {
#ifdef XMODEM_USE_UART1
				Serial.printf("To export file %s :\n", path+1);
				Serial.println("1) enter \"y\"");
				Serial.println("2) receive your file over the secondary serial port with :");
				Serial.printf("PORT=/dev/ttyUSB1 ; stty -F $PORT %lu && lrx -v /tmp/$$.tmp >$PORT <$PORT && tr -d '\\32' </tmp/$$.tmp >%s ; rm -f /tmp/$$.tmp\n", UserCmd::XMODEM_SERIAL_SPEED, path+1);
#else
				Serial.println("Warning: the Arduino Serial Monitor is not suitable for files transfers over the built-in USB port");
				Serial.println("use a serial terminal like minicom with : minicom -b 115200 -D /dev/ttyUSB0");
				Serial.printf("Procedure to export file %s :\n", path+1);
				Serial.println("1) enter \"y\" and close this terminal when prompted");
				Serial.println("2) receive your file with :");
				Serial.printf("PORT=/dev/ttyUSB0 ; stty -F $PORT %lu && lrx -v /tmp/$$.tmp >$PORT <$PORT && tr -d '\\32' </tmp/$$.tmp >%s ; rm -f /tmp/$$.tmp\n", UserCmd::XMODEM_SERIAL_SPEED, path+1);
				Serial.println("3) reopen the terminal");
#endif
				Serial.println("");
				char input=enter_one_char("Enter \"y\" to continue, or enter \"n\" to cancel");
				if (input=='y') {

#ifdef XMODEM_USE_UART1
					Serial.println("Receive your file now...");
#else
					Serial.println("Close this terminal now (minicom : Ctrl-A X)");
#endif
					int result=xmodem_export_file(path);
#ifndef XMODEM_USE_UART1
					enter_one_char("Press Enter to continue...");
#endif
					Serial.printf("\nTransfer of file %s ", path+1);
					if (result==0)
						Serial.println("successful");
					else
						Serial.printf("failed, error %d\n", result);
				}
				else
					Serial.println("Transfer canceled");
			}
			else
				Serial.printf("invalid file number %d\n", file_number);
			break;

		case 'c':
			if (strlen(arg2)>0) {
				*path='/';
				strncpy(path+1, arg2, MAXPATHLEN-1);
				File file_obj=LittleFS.open(path,"w");
				if (file_obj) {
					file_obj.close();
					Serial.printf("file %s created\n", path);
					exec_command('l', "", "", 0);
				}
				else
					Serial.println("open error");
			}
			break;

		case 'a':
			if (get_abs_path(file_number, FilesCount, path)==0) {
				// filter out whitespace in text
				File file_obj=LittleFS.open(path,"a");
				if (file_obj) {
					char *line=(char *)calloc(max_input_length+1, sizeof(char));
					if (line) {
						int textlen=strlen(arg3);
						strncpy(line, arg3, max_input_length);
						line[textlen]='\n';
						int count=file_obj.write((const uint8_t *)line, textlen+1);
						if (count==textlen+1)
							Serial.printf("file %s updated\n", path);
						else
							Serial.println("write error");
						file_obj.close();
						free(line);
					}
					else
						Serial.println("out of memory");
				}
				else
					Serial.println("open error");
			}
			else
				Serial.printf("invalid file number %d\n", file_number);
			break;

		case 'p':
			if (get_abs_path(file_number, FilesCount, path)==0) {
				Serial.println("--------------------");
				if (!print_file(path))
					Serial.print('\n'); // file not terminated by \n
				Serial.println("--------------------");
			}
			else
				Serial.printf("invalid file number %d\n", file_number);
			break;

		case 'D':
			if (get_abs_path(file_number, FilesCount, path)==0) {
				if (LittleFS.remove(path)) {
					Serial.printf("file %s deleted\n", path);
					exec_command('l', "", "", 0);
				}
				else
					Serial.println("remove error");
			}
			else
				Serial.printf("invalid file number %d\n", file_number);
			break;

		default:
			Serial.println("invalid command");
	}
	trprintf("*** %s %s() returns\n", __FILE_NAME__, __FUNCTION__);
}

// list the root directory and fill the array file_names_out[][]
// names of subdirectories are not copied in the array
// derived from listDir() in example LITTLEFS_test.ino
// return value: number of items copied in array
int UserCmd::list_root_dir(bool print_file_names, char file_names_out[][MAXNAMELEN]){
	trprintf("*** %s %s(%d, %d, %d) begin\n", __FILE_NAME__, __FUNCTION__, print_file_names, MAXFILES, MAXNAMELEN);
	int retval=0;
	for (int idx=0; idx<MAXFILES; idx++)
		*file_names_out[idx]='\0';
	File root = LittleFS.open("/");
    if(!root){
        Serial.println("- failed to open directory");
        return retval;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return retval;
    }
	File file_obj = root.openNextFile();
    while (file_obj && retval<MAXFILES){
		bool is_file=!file_obj.isDirectory();
		if (print_file_names) {
			if (is_file) {
				Serial.print("File #");
				Serial.print(retval+1);
				Serial.print(": ");
				Serial.print(file_obj.name());
				Serial.print("\tsize: ");
				Serial.println(file_obj.size());
			}
			else {
				Serial.print("Dir : ");
				Serial.println(file_obj.name());
			}
		}
		if (is_file) {
			strncpy(file_names_out[retval], file_obj.name(), MAXNAMELEN);
			retval++;
		}
        file_obj = root.openNextFile();
	}
	trprintf("*** %s %s() returns\n", __FILE_NAME__, __FUNCTION__);
	return retval;
}

// Receive text file with XModem protocol
// this protocols transmits fixed-size 128 bytes data packets
// and the last packet may be padded with SUB characters up to 128 bytes
// XModem::rx() removes the padding from the received file, so this method
// may not be suitable for receiving binary files.
int UserCmd::xmodem_import_file(const char *path) {
	trprintf("*** %s %s(%s) begin\n", __FILE_NAME__, __FUNCTION__, path);
	int retval=0; // success
	TransFile_obj=LittleFS.open(path,"w");
	if (TransFile_obj) {
		XModem xmodem_obj;
#ifdef XMODEM_USE_UART1
		// The RX and TX pins of UART1 are assigned to GPIO10 and GPIO9 by default
		// but they are are connected to the flash memory in the ESP32 board.
		// Hence, we will have to reassign the pins for UART1 for serial communication. 
		// The ESP32 board is capable to use almost all GPIO pins for serial connections,
		SerialPort1.begin(UserCmd::XMODEM_SERIAL_SPEED, SERIAL_8N1, UART1_RX_GPIO, UART1_TX_GPIO);
		xmodem_obj.begin(SerialPort1, XModem::ProtocolType::XMODEM);
#else
		Serial.flush();
		Serial.begin(UserCmd::XMODEM_SERIAL_SPEED, SERIAL_8N1);
		while (!Serial);
		Serial.flush();
		xmodem_obj.begin(Serial, XModem::ProtocolType::XMODEM);
#endif
		xmodem_obj.setRecieveBlockHandler(xmodem_write_block);
		// the the Arduino Serial Monitor is not suitable for files transfers over the built-in USB port :
		// xmodem_obj.receive() crashes as soon as user closes the Serial Monitor ; use minicom instead
		if (!xmodem_obj.receive())
			retval=4; // transfer failed
		TransFile_obj.close();
#ifndef XMODEM_USE_UART1
		Serial.flush();
		Serial.begin(UserCmd::CMD_SERIAL_SPEED, SERIAL_8N1);
		while (!Serial);
#endif
	}
	else {
		Serial.println("open error");
		retval=3;
	}
	trprintf("*** %s %s() returns %d\n", __FILE_NAME__, __FUNCTION__, retval);
	return retval;
}

bool UserCmd::xmodem_write_block(void *blk_id, size_t idSize, byte *buffer, size_t count) {
	trprintf("*** %s %s(%d bytes) begin\n", __FILE_NAME__, __FUNCTION__, count);
    bool retval=true;
	if (TransFile_obj.write((const uint8_t *)buffer, count) != count)
		retval=false; // write error
	trprintf("*** %s %s() returns %s\n", __FILE_NAME__, __FUNCTION__, retval?"true":"false");
    return retval;
}

// Send text file with XModem protocol,
// buffering whole file in RAM (300 KB available with my hardware)
// this protocols transmits fixed-size 128 bytes data packets
// and the last packet may be padded with SUB characters up to 128 bytes,
// so this method may not be suitable for transmitting binary files.
// You can remove the padding from the received text file with tr -d '\32' <received_file.txt >filtered_file.txt
int UserCmd::xmodem_export_file(const char *path) {
	trprintf("*** %s %s(%s) begin\n", __FILE_NAME__, __FUNCTION__, path);
	int retval=0; // success
	TransFile_obj=LittleFS.open(path,"r");
	if (TransFile_obj) {
		size_t file_size=TransFile_obj.size();
		size_t buffer_size=file_size;
		char *buffer=(char *)calloc(buffer_size, sizeof(char));
		if (buffer) {
			XModem xmodem_obj;
#ifdef XMODEM_USE_UART1
		// The RX and TX pins of UART1 are assigned to GPIO10 and GPIO9 by default
		// but they are are connected to the flash memory in the ESP32 board.
		// Hence, we will have to reassign the pins for UART1 for serial communication. 
		// The ESP32 board is capable to use almost all GPIO pins for serial connections,
		// here we have reassigned GPIO25 as RX pin and GPIO26 as TX pin.
		SerialPort1.begin(UserCmd::XMODEM_SERIAL_SPEED, SERIAL_8N1, UART1_RX_GPIO, UART1_TX_GPIO);
		xmodem_obj.begin(SerialPort1, XModem::ProtocolType::XMODEM);
#else
			Serial.flush();
			Serial.begin(UserCmd::XMODEM_SERIAL_SPEED, SERIAL_8N1);
			while (!Serial);
			Serial.flush();
			xmodem_obj.begin(Serial, XModem::ProtocolType::XMODEM);
#endif			
			int read_len=TransFile_obj.readBytes(buffer, buffer_size);
			if (read_len>0) {
				if (!xmodem_obj.send((byte *)buffer, buffer_size))
					retval=4;
			}
			free(buffer);
			TransFile_obj.close();
#ifndef XMODEM_USE_UART1
			Serial.flush();
			Serial.begin(UserCmd::CMD_SERIAL_SPEED, SERIAL_8N1);
			while (!Serial);
#endif
		}
		else {
			Serial.println("out of memory");
			retval=2;
		}
	}
	else {
		Serial.println("open error");
		retval=3;
	}
	trprintf("*** %s %s() returns %d\n", __FILE_NAME__, __FUNCTION__, retval);
	return retval;
}

// prints the raw contents of the file on Serial
// path_str	must start with '/' (absolute path)
// return value: true if the last char of the file is a '\n'
bool UserCmd::print_file(const char *path_str) {
	bool retval=false;
	Serial.printf("file %s :\n", path_str);
	File file_obj=LittleFS.open(path_str,"r");
	if (file_obj) {
		const int BLOCK_LENGTH=255;
		const int BUFFER_SIZE=BLOCK_LENGTH+1;
		char *buffer=(char *)calloc(BUFFER_SIZE, sizeof(char));
		if (buffer) {
			while (file_obj.available()) {
				memset(buffer, 0, BUFFER_SIZE);
				int read_len=file_obj.readBytes(buffer, BLOCK_LENGTH);
				if (read_len>0) {
					Serial.print(buffer);
					retval=(buffer[read_len-1]=='\n');
				}
			}
			free(buffer);
		}
		else
			Serial.println("out of memory");
		file_obj.close();
	}
    else
		Serial.printf("file not found %s\n", path_str);
	return retval;
}

int UserCmd::get_abs_path(int file_number, int files_count, char *path_out) {
	int retval=0; //success
	if (file_number>=1 && file_number<=files_count) {
		*path_out='/';
		strncpy(path_out+1, FileNames[file_number-1], MAXPATHLEN-1);
	}
	else
		retval=1; // error
	return retval;
}

/* Serial I/O ****************************************************************/

// Acquire bytes received through Serial until CR or LF or ESC, blocking
// CR and LF are replaced by \0 in given buffer
// max_input_length >0 and < sizeof(input_buffer_out) because a \0 will be appended to the user input
// Return value: number of characters received before CR or LF, or USER_CANCEL if received ESC
int UserCmd::serial_input(char *input_buffer_out, size_t max_input_length) {
	int retval=0;
	int input_count=0;
	bool input_complete=false;
	while (!input_complete) {
		if (Serial.available()) {
			char input_chr = Serial.read();
			switch(input_chr) {
				case 10: // LF
				case 13: // CR
					input_buffer_out[input_count]='\0';
					input_complete=true;
					retval=input_count;
					// clear next character(s) from the read buffer,
					// possibly a LF following the CR which triggered this case 
					while (Serial.available())
						Serial.read();
					break;

				case 27: // Escape
					input_buffer_out[0]='\0';
					input_complete=true;
					retval=USER_CANCEL; // user cancelled input
					break;

				case 8: // Backspace
					if (input_count>0) {
						input_count--;
						Serial.print(input_chr);
					}
					break;

				default:
					if (input_count<max_input_length) {
						input_buffer_out[input_count++]=input_chr;
						Serial.print(input_chr);
					} // else ignore extra input
					break;
			}
		}
	}
	//Serial.print('\n');
	return retval;
}

// return value: number of characters received
// size_t UserCmd::serial_import(char *buffer_out, size_t max_file_size) {
// 	size_t offset=0;
// 	memset(buffer_out, 0, max_file_size);
// 	unsigned long timeout=0;

// 	// wait for the 1st byte
// 	const unsigned START_TIMEOUT=60000; // ms ; allow 1 minute to select the file and start the transfer
// 	timeout=millis()+START_TIMEOUT;
// 	while (millis()<timeout) {
// 		if (Serial.available()) {
// 			buffer_out[offset++]=Serial.read();
// 			break;
// 		}
// 	}
// 	if (offset==1) {
// 		// read serial input until nothing received for READ_TIMEOUT
// 		const unsigned READ_TIMEOUT=500; // ms ; not receiving any byte during this delay means we have reached EOF
// 		timeout=millis()+READ_TIMEOUT;
// 		while (millis()<timeout) {
// 			if (offset<max_file_size) {
// 				if (Serial.available()) {
// 					buffer_out[offset++]=Serial.read();
// 					timeout=millis()+READ_TIMEOUT;
// 				}
// 			}
// 			else
// 				break; // read buffer full
// 		}
// 	}
// 	return offset;
// }

void UserCmd::serial_prompt(void) {
	Serial.println("ready");
}

// return value: the character typed, or USER_CANCEL=Escape
int UserCmd::enter_one_char(const char *message) {
	int retval=USER_CANCEL;
	char buffer[2];
	Serial.println(message);
	if (serial_input(buffer, 1)==1) {
		Serial.println();
		retval=buffer[0];
	}
	return retval;
}

