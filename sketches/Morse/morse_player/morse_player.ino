/* morse_player.ino - Morse audio player
**
**	|--------|Piezo buzzer|------< Digital output pin
**	|
**	|----------|120 Ohm|----------o GND
**
** 13-02-2012
*/

/*
** Resources -----------------------------------------------
*/
#include <rgUI.h>
#include <rgMorsePlayer.h>

/*
** User interface declarations -----------------------------------------------
*/

rgUI UI_obj;
const byte CMD1='a'; // play audio
const byte CMD2='r'; // flash led
const byte CMDEXIT=27; // escape
const byte CMDENTER=13; // return

const int MODE_IDLE=0;
const int MODE1=1;
const int MODE2=2;
int State_int=MODE_IDLE;

/*
** Implementation declarations -----------------------------------------------
*/

rgMorsePlayer Player_obj;
const int TEXTSIZE = 80;
char Text_str[TEXTSIZE];

// Liste des 610 mots de 3 lettres
// http://www.listes.ortograf.com/mots33.htm
const int NWORDS = 610;
char *Words_str[] = {"AAS","ACE","ADA","ADO","AGA","AGE","AGI","AID","AIE","AIL","AIR","AIS","AIT","ALE","ALU","AME","AMI","ANA","ANE","ANI","ANS","API","ARA","ARC","ARE","ARS","ART","ASA","ASE","AUX","AVE","AXA","AXE","AYS","BAC","BAH","BAI","BAL","BAN","BAR","BAS","BAT","BAU","BEA","BEC","BEE","BEL","BEN","BER","BEY","BIC","BIO","BIP","BIS","BIT","BLE","BOA","BOB","BOF","BOG","BOL","BON","BOP","BOT","BOX","BOY","BRU","BUE","BUG","BUN","BUS","BUT","BYE","CAB","CAF","CAL","CAP","CAR","CAS","CEP","CES","CET","CHU","CIF","CIL","CIS","CLE","COB","COI","COL","COM","CON","COQ","COR","COU","COX","CRE","CRI","CRU","CUL","CUT","DAB","DAH","DAL","DAM","DAN","DAO","DAW","DEB","DEO","DER","DES","DEY","DIA","DIN","DIS","DIT","DIX","DOC","DOL","DOM","DON","DOP","DOS","DOT","DRU","DRY","DUB","DUC","DUE","DUO","DUR","DUS","DUT","DZO","EAU","ECO","ECU","EGO","ELU","EMU","EON","EPI","ERE","ERG","ERS","EST","ETA","ETE","EUE","EUH","EUS","EUT","EUX","EWE","EXO","FAC","FAF","FAN","FAQ","FAR","FAT","FAX","FEE","FER","FEU","FEZ","FIA","FIC","FIE","FIL","FIN","FIS","FIT","FLA","FOB","FOC","FOG","FOI","FOL","FON","FOR","FOU","FOX","FUI","FUN","FUR","FUS","FUT","GAG","GAI","GAL","GAN","GAP","GAY","GAZ","GEL","GEO","GEX","GIN","GIS","GIT","GLU","GOI","GON","GOS","GOY","GRE","GUE","GUI","GUR","GUS","GYM","HAI","HAN","HEM","HEP","HEU","HIA","HIC","HIE","HIP","HIT","HOP","HOT","HOU","HUA","HUB","HUE","HUI","HUM","HUN","IBN","IBO","ICI","IDE","IFS","ILE","ILS","ION","IPE","IRA","IRE","ISO","IVE","IXA","IXE","JAB","JAM","JAN","JAR","JAS","JET","JEU","JOB","JUS","KAN","KAS","KAT","KEA","KEN","KET","KHI","KID","KIF","KIL","KIP","KIR","KIT","KOB","KOI","KOP","KOT","KRU","KSI","KWA","KYU","LAC","LAD","LAI","LAO","LAS","LED","LEI","LEK","LEM","LES","LET","LEU","LEV","LEZ","LIA","LIE","LIN","LIS","LIT","LOB","LOF","LOG","LOI","LOS","LOT","LUE","LUI","LUO","LUS","LUT","LUX","LYS","MAC","MAI","MAL","MAN","MAO","MAS","MAT","MAX","MEC","MEL","MEO","MER","MES","MET","MIE","MIL","MIN","MIR","MIS","MIT","MIX","MMM","MOA","MOB","MOI","MOL","MON","MOR","MOS","MOT","MOU","MOX","MUA","MUE","MUG","MUR","MUS","MUT","MYE","NAC","NAN","NAY","NEE","NEF","NEM","NEO","NES","NET","NEY","NEZ","NIA","NIB","NID","NIE","NIF","NIM","NIT","NOM","NON","NOS","NUA","NUE","NUI","NUL","NUS","OBA","OBI","ODE","OFF","OHE","OHM","OIE","OIL","OKA","OLA","OLE","ONC","ONT","OPE","ORE","ORS","OSA","OSE","OST","OTA","OTE","OUD","OUF","OUH","OUI","OUT","OVE","OXO","OYE","PAF","PAL","PAN","PAP","PAR","PAS","PAT","PEC","PEP","PET","PEU","PFF","PHI","PHO","PIC","PIE","PIF","PIN","PIS","PIU","PLI","PLU","POP","POT","POU","PRE","PRO","PSI","PST","PSY","PUA","PUB","PUE","PUR","PUS","PUT","PUY","QAT","QIN","QUE","QUI","RAB","RAC","RAD","RAI","RAM","RAP","RAS","RAT","RAY","RAZ","REA","REE","REG","REM","REZ","RHE","RHO","RIA","RIE","RIF","RIO","RIS","RIT","RIZ","ROB","ROC","ROI","ROM","ROS","ROT","RUA","RUE","RUS","RUT","RUZ","RYE","SAC","SAI","SAL","SAR","SAS","SAX","SEC","SEL","SEN","SEP","SES","SET","SIC","SIL","SIR","SIS","SIX","SKA","SKI","SOC","SOI","SOL","SOM","SON","SOT","SOU","SPA","SPI","SUA","SUC","SUD","SUE","SUP","SUR","SUS","SUT","TAC","TAF","TAG","TAN","TAO","TAR","TAS","TAT","TAU","TEC","TEE","TEK","TEL","TEP","TER","TES","TET","TEX","THE","TIC","TIF","TIN","TIP","TIR","TOC","TOF","TOI","TOM","TON","TOP","TOS","TOT","TRI","TUA","TUB","TUE","TUF","TUS","TUT","UDS","UNE","UNI","UNS","URE","USA","USE","UTE","VAL","VAN","VAR","VAS","VAU","VER","VES","VET","VIA","VIE","VIF","VIL","VIN","VIS","VIT","VOL","VOS","VUE","VUS","WAD","WAP","WAX","WEB","WOH","WOK","WON","WUS","YAK","YAM","YEN","YET","YIN","YOD","YUE","ZEC","ZEE","ZEF","ZEK","ZEN","ZIG","ZIP","ZOB","ZOE","ZOO","ZOU","ZUP","ZUT"};

/*
** Main code -----------------------------------------------------------------
*/

void setup() {
	// initialize user interface
	UI_obj.Title("morse_player v1.0 - Morse audio player", "v1.0");
	print_usage();
	UI_obj.Prompt();
	
	randomSeed(analogRead(0));
}

void loop() {
	if (State_int == MODE2) {
		static int count_int = 0;
		static char *word_str = NULL;
		
		if (count_int == 0)
			word_str = Words_str[random(0, NWORDS-1)];
		
		Player_obj.Play(word_str);
		delay(2000);
		Serial.print(word_str); Serial.print(" ");
		
		if (++count_int == 5) {
			count_int = 0;
			Serial.println("");
		}
	}
}

/*
** User interface ------------------------------------------------------------
*/

void print_usage() {
	Serial.println("");
	Serial.println("Enter command:");
	Serial.println("  'a' to play typed string");
	Serial.println("  'r' to play random words");
	Serial.println("  'Esc' to exit to this Menu");
	UI_obj.Prompt();
}
	
/* SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
	char echo_str[40];
	while (Serial.available()) {
		// get the new byte
		byte command_byte = Serial.read();
		// echo user input
		if (command_byte >= ' ') {
			sprintf(echo_str, "%c", command_byte);
			Serial.print(echo_str);
		}
		// parse input
		if (State_int == MODE_IDLE) {
			switch (command_byte) {
				
				case CMD1: // Play typed string
					State_int=MODE1;
					break;
				
				case CMD2: // play random words
					State_int=MODE2;
					break;
				
				default:
					// nop
					break;
			}
			memset(Text_str, '\0', TEXTSIZE);
		}
		else {
			switch (command_byte) {
				
				case CMDEXIT:
					// return to menu
					State_int=MODE_IDLE;
					Serial.println("");
					print_usage();
					break;
				
				case CMDENTER:
					if (State_int == MODE1) {
						// play the contents of the Text_str buffer
						Serial.println("");
						UI_obj.Prompt();
						// notice: we clear the Text_str buffer before playing because user may type new characters while it is playing
						char text_dup_str[TEXTSIZE];
						strcpy(text_dup_str, Text_str);
						memset(Text_str, '\0', TEXTSIZE);
						// play the copy of the text buffer
						Player_obj.Play(text_dup_str);
					}
					break;
				
				default:
					if (State_int == MODE1) {
						// store this char in the Text_str buffer
						Text_str[strlen(Text_str)] = command_byte;
					}
					break;
			}
		}
	}
}

