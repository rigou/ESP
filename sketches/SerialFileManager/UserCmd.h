/* UserCmd.h - Example of a user-defined command interpreter
 * This program is published under the GNU General Public License. 
 * This program is free software and you can redistribute it and/or modify it under the terms
 * of the GNU General  Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.
 * See the GNU General Public License for more details : https://www.gnu.org/licenses/ *GPL 
 *
 * Installation, usage : https://github.com/rigou/nRF24L01-FHSS/
*/

#pragma once

#define CMDLIB_NAME "UserCmd"
#define CMDLIB_VERSION "v1.5.5"
#define XMODEM_USE_UART1	// uncomment this line to use the secondary serial port (RX1=GPIO 25, TX1=GPIO 26) for file transfers

class UserCmd {
	private:

		static const int USER_CANCEL=-1;

		static const int MAXFILES=20;
		static const int MAXNAMELEN=14; // including the trailing zero, file names format : [/]8.3
		static const int MAXPATHLEN=14; // this version manages files in the root directory only
		char FileNames[MAXFILES][MAXNAMELEN];
		int FilesCount=0;
			
		void parse_command_line(char *command_str, char *arg1_out, char *arg2_out, char *arg3_out, size_t max_input_length);
		void exec_command(char arg1, const char *arg2, const char *arg3, size_t max_input_length);
		int list_root_dir(bool print_file_names, char file_names_out[][MAXNAMELEN]);
		int xmodem_import_file(const char *path);
		int xmodem_export_file(const char *path);
		static bool xmodem_write_block(void *blk_id, size_t idSize, byte *buffer, size_t count);
		bool print_file(const char *path_str);
		int get_abs_path(int file_number, int files_count, char *path_out);
		int serial_input(char *input_buffer_out, size_t max_input_length);
		//size_t serial_import(char *buffer_out, size_t max_file_size);
		void serial_prompt(void);
		int enter_one_char(const char *message);
				
	public:
	    // if the ESP32 board has a CP2102 USB-TO-UART bridge, 
    	// minicom handles 921600 bauds but Arduino serial monitor 500000 bauds only
		static const uint32_t CMD_SERIAL_SPEED=115200; // use legacy arduino baud rate for interactive commands
		static const uint32_t XMODEM_SERIAL_SPEED=500000; // up to 921600 bauds for a CP2102 or 3000000 bauds for a FT232R

		void Setup(void);
		int Input(size_t max_input_length);
		void PrintHelp(void);
};
