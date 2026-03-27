/* CsvTest
** 2024-10-04
**
** Requirements:
** 	1) partition: Arduino IDE/Tools/Partition scheme/Default 4MB withs spiffs
**  2) filesystem: LittleFS filesystem, already formatted
*/

#include "rgCsv.h"

/*****************************************
* Application parameters file param.csv
*****************************************/
enum class Paramid {
	TXID,
	MONOCHAN,
	PALEVEL
};
rgCsv ParamCsv_obj;
// we use these macros to simplify access to the mCells array of ParamCsv_obj
#define PARAMGETINT(x)	ParamCsv_obj.GetIntCell((byte)Paramid::x,1)
#define PARAMSETINT(x,v)	ParamCsv_obj.SetIntCell((byte)Paramid::x, 1, (v))


/*****************************************
* User parameters file usrparam.csv
******************************************
# DFMOD Default model file name
# BAT	Minimum voltage required for Tx operation, an alarm is triggered when voltage gets
#   	lower. ESP-WROOM-32 requires 3.3V and the AMS1117 max dropout is 1.3V so min VBat=4.6V
#   	- set BAT=4600 for a 5V power supply
#   	- set BAT=7500 for a LiPo 2S battery to prevent over-discharge
#
*/
enum class UParamid {
	DEFMOD,
	BATMIN
};
rgCsv UParamCsv_obj;
// we use these macros to simplify access to the mCells array of UParamCsv_obj
#define UPARAMGETINT(x)		UParamCsv_obj.GetIntCell(byte(UParamid::x),1)
#define UPARAMSETINT(x,v)	UParamCsv_obj.SetIntCell((byte)UParamid::x, 1, (v))
#define UPARAMGETSTR(x)		UParamCsv_obj.GetStrCell(byte(UParamid::x),1)
#define UPARAMSETSTR(x,v)	UParamCsv_obj.SetStrCell((byte)UParamid::x, 1, (v))

/******************************************
* Model configuration file usrmodel.csv
*******************************************
* --------------------
* Global configuration
* --------------------
# CHA	0 : channel number 0 is reserved for global variables
# NAME  Model name
# CHN	Number of controlled channels
# THC	Throttel channel, value in the range [1-CHA]
# TSC	throttle cutoff value used by the throttle security check, default = 50	
*/
enum class Uglid {
	CHA,
	NAME,
	CHN,
	THC,
	TSC
};
/* ---------------------
* Channels configuration
* ----------------------
# CHA	Channel number
# NAME  Channel name
# ICT	Input control type: 1=potentiometer, 2=switch, 0=not used
# ICN	GPIO number of the pot/switch controlling this channel, defined in User.h
# ANL	lowest analog value read on the potentiometer, when the stick (and the hardware trim,
#		if any) is at its lowest position. Value inthe range [0-1023] default = 0
# ANH	Highest analog value read on the potentiometer, when the stick (and the hardware trim,
#		if any) is at its highest position. Value inthe range [0-1023] default = 1023
# REV	Reverse, 1=servo direction is reversed for this channel, 0=servo is not reversed.
# DUA	Dual rate applied to this channel when the Dual Rate switch is ON. Percentage [0-100],
#		Applies only if EXP=0
# EXP	Exponential applied symetrically from the center of this channel when the Dual Rate
#		switch is ON. Percentage [0-100]. 0=none, 25=medium, 50=strong, 100=very strong.
#		If EXP is not zero then DUA is ignored. Exponential may also apply to the throttle
#		channel (variable THC). This feature may be useful for gas engines. This exponential
#		is calculated full-curve from 0
# PWL	Minimal pulse width for this channel, in microseconds, default=720
#		This default value corresponds to the Hextronic HXT500 servo
# PWH	Maximal pulse width for this channel, in microseconds, default=2200
#		This default value corresponds to the Hextronic HXT500 servo
# EPL	Low end point position in percentage from the center of the channel [0,100],
#		default = 100. Set this variable if you want to limit the rotation angle of the servo
# EPH	High end point position in percentage from the center of the channel [0,100],
#		default = 100. Set this variable if you want to limit the rotation angle of the servo
# SUB	Subtrim signed percentage [-100,+100], default=0. Setting this variable shifts the
#		neutral position of the servo in either direction, depending on the sign of the value:
#		negative shifts left, positive shifts right
*/
enum class Uchid {
	CHA,
	NAME,
	ICT,
	ICN,
	ANL,
	ANH,
	REV,
	DUA,
	EXP,
	PWL,
	PWH,
	EPL,
	EPH,
	SUB
};

rgCsv UserCsv_obj;
// we use these macros to simplify access to the mCells array of UserCsv_obj
// examples: UGLGETINT(CHN) returns the Number of controlled channels, UCHGETINT(ICT, 2) returns the Input control type of channel #2
#define UGLGETINT(x)		UserCsv_obj.GetIntCell(0,(byte)Uglid::x)
#define UGLSETINT(x,v)		UserCsv_obj.SetIntCell(0,(byte)Uglid::x,(v))
#define UGLGETSTR(x)		UserCsv_obj.GetStrCell(0,(byte)Uglid::x)
#define UGLSETSTR(x,v)		UserCsv_obj.SetStrCell(0,(byte)Uglid::x,(v))
#define UCHGETINT(x,y)		UserCsv_obj.GetIntCell((y),(byte)Uchid::x)
#define UCHSETINT(x,y,v)	UserCsv_obj.SetIntCell((y),(byte)Uchid::x,(v))
#define UCHGETSTR(x,y)		UserCsv_obj.GetStrCell((y),(byte)Uchid::x)
#define UCHSETSTR(x,y,v)	UserCsv_obj.SetStrCell((y),(byte)Uchid::x,(v))


void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect   
	Serial.print('\n'); for (uint8_t idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } // debug
	Serial.printf("\nusing library %s %s\n", CSVLIB_NAME, CSVLIB_VERSION);

	Serial.println("read the application parameters");
	int params_int=0;
	int result_int=ParamCsv_obj.Allocate("param.csv", 3, 2, 12, false);
	if (result_int==0) {
		params_int=ParamCsv_obj.Load();
		if (params_int>0) {
			Serial.printf("param TXID=%u\n", (uint16_t)PARAMGETINT(TXID));
			Serial.printf("param MONOCHAN=%d\n", PARAMGETINT(MONOCHAN));
			Serial.printf("param PALEVEL=%d\n", PARAMGETINT(PALEVEL));
			// we do not call ParamCsv_obj.Release() because IRL we'll need to access these values later
		}
		else {
			Serial.printf("Load(param) failed, error %d\n", params_int);
			ParamCsv_obj.Release();
		}
	}
	else
		Serial.printf("Allocate(param) failed, error %d\n", result_int);
	if (params_int<=0) {
		Serial.printf("setup End (%d)\n", params_int);
		while(1);
	}

	Serial.println("read the user parameters");
	params_int=0;
	result_int=UParamCsv_obj.Allocate("usrparam.csv", 2, 2, 12, false);
	if (result_int==0) {
		params_int=UParamCsv_obj.Load();
		if (params_int>0) {
			Serial.printf("param DEFMOD=%s\n", UPARAMGETSTR(DEFMOD));
			Serial.printf("param BATMIN=%d\n", UPARAMGETINT(BATMIN));
			// we do not call UParamCsv_obj.Release() because IRL we'll need to access these values later
		}
		else {
			Serial.printf("Load(usrparam) failed, error %d\n", params_int);
			ParamCsv_obj.Release();
		}
	}
	else
		Serial.printf("Allocate(usrparam) failed, error %d\n", result_int);
	if (params_int<=0) {
		Serial.printf("setup End (%d)\n", params_int);
		while(1);
	}
}

#define MAXLINES 10		// max number of data lines in the csv file (not counting blank lines and comments)
#define MAXCELLS 14 	// max number of cells per line
#define MAXCELLENGTH 20	// max number of characters per cell (including leading and trailing spaces, if any)
void loop() {
	Serial.println("read the model configuration");
	const char *model_file=UPARAMGETSTR(DEFMOD);
	int result_int=UserCsv_obj.Allocate(model_file, MAXLINES, MAXCELLS, MAXCELLENGTH, false);
	if (result_int==0) {
		result_int=UserCsv_obj.Load();
		if (result_int>0) {
			Serial.printf("file %s : %s, %d channels\n", model_file, UGLGETSTR(NAME), UGLGETINT(CHN));
			print_channels();

			// change some values in channel 1
			//UserCsv_obj.SetIntCell(1, (byte)Uchid::SUB, UCHGETINT(SUB,1)+1);
			UCHSETINT(SUB, 1, UCHGETINT(SUB,1)+1);
			char name_str[10];
			snprintf(name_str, 9, "Motor_%d", UCHGETINT(SUB,1));
			//UserCsv_obj.SetStrCell(1, (byte)Uchid::NAME, name_str);
			UCHSETSTR(NAME, 1, name_str);

			print_channels();
			UserCsv_obj.Save();
		}
		else
			Serial.printf("Load(user) failed, error %d\n", result_int);

		UserCsv_obj.Release();
	}
	else
		Serial.printf("Allocate(user) failed, error %d\n", result_int);

	Serial.println("loop End");
	while(1);
}

void print_channels() {
	Serial.println("");
	for (int idx=1; idx<=UGLGETINT(CHN); idx++)
		Serial.printf("\tchannel %d : %s, gpio %d, subtrim=%d\n", idx, UCHGETSTR(NAME, idx), UCHGETINT(ICN, idx), UCHGETINT(SUB, idx));
}
