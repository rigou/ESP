/* FParam.ino - tests for class rgParam
   to trace execution: #define RGPARAM_TRACE in rgParam.h
*/

#include "rgParam.h"

rgParam FPar_obj;

void setup() {
    Serial.begin(921600);
	while (!Serial) ; // wait for serial port to connect
	// give user some time to open the serial monitor
	Serial.println("");
	int idx = 0;
	while (idx < 12) {
		Serial.print((char)('A'+idx++));
		delay(500);
	}
    Serial.printf("\n%s %s\n", PARAMLIB_NAME, PARAMLIB_VERSION);
}

void loop() {
	const char *path_str="testpara.txt";
	char version_str[20];
	static unsigned int Count_int=0;

	// Init() args: format_fs_if_failed, max_len_path, max_len_key, max_len_value, max_records
	FPar_obj.Init(path_str, 1, 16, 10, 8, 20);
	FPar_obj.SetSeparator('=');

	//LittleFS.remove(path_str);

	static char File_buff[10000];
	FPar_obj.Dump(NULL, sizeof(File_buff), File_buff);

	FPar_obj.Load();
	FPar_obj.GetKeyStr("Version", version_str);
	FPar_obj.GetKeyInt("Counter", &Count_int);

	Serial.print('\n');
	Serial.println("-----------------------------------");
	Serial.println("raw data");
	Serial.print(File_buff);
	Serial.println("-----------------------------------");
	FPar_obj.SerialDump();
	Serial.println("-----------------------------------");
	Serial.println("test results");
	Serial.printf("Version=\"%s\"\n", version_str);
	Serial.printf("Counter=0x%x (dec %u)\n", Count_int, Count_int);
	Serial.println("-----------------------------------");
	Serial.println("Press Enter to increment the counter...");
	while (!press_key('\n'));

	FPar_obj.SetKeyStr("Version", PARAMLIB_VERSION);
	FPar_obj.SetKeyInt("Counter", ++Count_int);
	FPar_obj.Save();
}

bool press_key(char this_chr) {
	while (!Serial.available());
	return (Serial.read()==this_chr);
}