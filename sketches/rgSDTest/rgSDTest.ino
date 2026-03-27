/* SD card read/write test
** 2013-03-25 22:28:31
** 2013-04-09 added debug msg
** 2023-11-22 ported to ESP32
*/

#include <SPI.h>
#include <SD.h>
#include "rgDFile.h"

#define WEMOS_LOLIN_32_LITE // inverted logic
#ifdef WEMOS_LOLIN_32_LITE
#define LED_BUILTIN 22
#endif
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

char DFPATH[] = "/SDTEST.TMP";
rgDFile DFile_obj(DFPATH);

char FLOG[] = "/SDTEST.LOG";
File Log_obj;

boolean SDInit_bool = false;

// Return elapsed time since startup in [h]hh:mm:ss format
// out_buffer_str:  user-allocated 10 chars minimum
char *get_time(char *out_buffer_str) {
	unsigned long time_lng = millis() / 1000L;
	byte hours_int = time_lng / 3600;
	byte minutes_int = (time_lng - (hours_int * 3600)) / 60;
	byte seconds_int = time_lng - (hours_int * 3600) - (minutes_int * 60);
	sprintf(out_buffer_str, "%02d:%02d:%02d", hours_int, minutes_int, seconds_int);
	return out_buffer_str;
}

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	// give user some time to open the serial monitor
	int count_int=0;
	Serial.println("");
	while (count_int<10) {
		Serial.printf("%c", 'A'+count_int++);
		delay(500);
	}
	Serial.println("");

	// provide visual feedback of SD activity: On=SD i/o in progress, Off=SD safe to remove
	pinMode(LED_BUILTIN, OUTPUT);
	
	Serial.println("Initializing SD card");
	Serial.print("MOSI: ");
	Serial.println(MOSI);
	Serial.print("MISO: ");
	Serial.println(MISO);
	Serial.print("SCK: ");
	Serial.println(SCK);
	Serial.print("CS: ");
	Serial.println(SS);
	Serial.println("5V: Vin");
	Serial.println("GND: Gnd");
	if (SD.begin(SS)) {
		SDInit_bool = true;
		DFile_obj.Delete();
	}
	else {
		Serial.println("initialization failed.");
		while(1);
	}

	Serial.println("You can safely remove the SD card when the built-in LED is off");
}

void loop() {
	const word NRECORDS = 10000;
	const word RECSIZE = 20;
	static unsigned long iterations_lng = 0;
	static unsigned long failed_iterations_lng = 0;
	static unsigned long open_errcount_lng = 0;
	static unsigned long write_errcount_lng = 0;
	static unsigned long read_errcount_lng = 0;
	boolean failure_bool=false;
	
	if (SDInit_bool) { // else nop
		char record_str[RECSIZE + 1];
		++iterations_lng;
#ifdef WEMOS_LOLIN_32_LITE
		digitalWrite(LED_BUILTIN, LOW);
#else
		digitalWrite(LED_BUILTIN, HIGH);
#endif
		// Write
		byte retval_byt = DFile_obj.Open(DFPATH, NRECORDS, RECSIZE, DFSTATE_WRITE);
		if (retval_byt == 0) {
			for (word reccount_int = 0; reccount_int < NRECORDS; reccount_int++)  {
				sprintf(record_str, "%d", reccount_int);
				if (reccount_int % (NRECORDS/10) == 0) {
					Serial.print("Write record ");
					Serial.println(record_str);
				}
				retval_byt = DFile_obj.Write(reccount_int, record_str, strlen(record_str));
				if (retval_byt != 0) {
					Serial.print("Write() error ");
					Serial.println(retval_byt);
					failure_bool = true;
					write_errcount_lng++;
				}
			}
			DFile_obj.Close();
		}
		else {
			Serial.print("Open(DFSTATE_WRITE) error ");
			Serial.println(retval_byt);
			failure_bool = true;
			open_errcount_lng++;
		}
		
		// Read
		retval_byt = DFile_obj.Open(DFPATH, NRECORDS, RECSIZE, DFSTATE_READ);
		if (retval_byt == 0) {
			for (word reccount_int = 0; reccount_int < NRECORDS; reccount_int++)  {
				if (reccount_int % (NRECORDS/10) == 0) {
					Serial.print("Read record ");
					Serial.println(reccount_int);
				}
				retval_byt = DFile_obj.Read(reccount_int, record_str, RECSIZE);
				record_str[RECSIZE] = '\0';
				if (retval_byt == 0) {
					if ((word)atoi(record_str) != reccount_int) {
						Serial.print("Invalid record ");
						Serial.println(reccount_int);
						failure_bool = true;
						read_errcount_lng++;
					}
				}
				else {
					Serial.print("Read() error ");
					Serial.println(retval_byt);
					failure_bool = true;
					read_errcount_lng++;
				}
			}
			DFile_obj.Close();
		}
		else {
			Serial.print("Open(DFSTATE_READ) error ");
			Serial.println(retval_byt);
			failure_bool = true;
			open_errcount_lng++;
		}
		
		if (failure_bool)
			failed_iterations_lng++;
		
		// format results
		char linebuff_str[100];
		get_time(linebuff_str); // 8 chars hh:mm:ss
		sprintf(linebuff_str+8, " %03lu iterations -> failed: %lu open err: %lu read err: %lu write err: %lu", 
			iterations_lng, failed_iterations_lng, open_errcount_lng, read_errcount_lng, write_errcount_lng);
		// append results to Log file
		Log_obj = SD.open(FLOG, FILE_WRITE);
		if (Log_obj.size() > 0)
			Log_obj.seek(Log_obj.size());
		Log_obj.println(linebuff_str);
		Log_obj.close();
		
		Serial.println(linebuff_str);
#ifdef WEMOS_LOLIN_32_LITE
		digitalWrite(LED_BUILTIN, HIGH);
#else
		digitalWrite(LED_BUILTIN, LOW);
#endif
		delay(5000);
	}
}
