// InputArgs.ino -  Simple program showing how to read some parameters from the Serial link
// Author: Richard Goutorbe

#define APP_NAME "InputArgs"
#define APP_VERSION "1.0.0"

uint16_t TxDeviceId=0;
uint8_t Channel=0;
uint8_t DatagramSize=0;

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	Serial.print('\n'); for (byte idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } ; Serial.print('\n');
	Serial.printf("\n%s %s\n", APP_NAME, APP_VERSION);

	TxDeviceId = serial_prompt("Type the Tx device id (decimal)");
	Channel = serial_prompt("Type the radio channel (decimal)");
	DatagramSize = serial_prompt("Type the size of the datagram (16-bit words)");
	serial_prompt("Press the Enter key to start");
}

int serial_prompt(const char *text) {
	int retval=0;
	char buffer[16];
	Serial.println(text);
	int count=serial_input(buffer, sizeof(buffer)-1);
	if (count>0) {
		sscanf(buffer, "%d", &retval);
		Serial.println();
	}
	return retval;
}

void loop() {
	Serial.println("ok");
	while(true);
}

// Acquire bytes received through Serial until CR or LF or ESC, blocking
// CR and LF are replaced by \0 in given buffer
// max_input_length < sizeof(input_buffer_out) because a \0 will be appended to the user input
// Return value: number of characters received before CR or LF, or USER_CANCEL if received ESC
const int USER_CANCEL = -1;
int serial_input(char *input_buffer_out, size_t max_input_length) {
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
