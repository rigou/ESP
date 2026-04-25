/* rgMorse.cpp - Morse decoder
** 19-01-2012 v1.1 adaptative ambient value
*/

#include "rgMorse.h"

const int rgMorse::DITTIME=150; // (ms) duration of a morse dot; dash duration >=  3*DITTIME
const int rgMorse::EVENTS=16; // max number of items in array Events_int[]
const int rgMorse::THRESHOLD = 50; // input signal is HIGH when > Ambient_int + THRESHOLD
const int  rgMorse::CLOCKPERIOD = 10; // (ms) the Clock() function does something every CLOCKPERIOD ms, else it just returns without doing any processing

rgMorse::rgMorse(const int pin_int)  {
	// Morse code, order by length
	Code_str[0]="E.|T-|";
	Code_str[1]="I..|A.-|N-.|M--|";
	Code_str[2]="S...|U..-|R.-.|W.--|D-..|K-.-|G--.|O---|";
	Code_str[3]="H....|V...-|F..-.|L.-..|P.--.|J.---|B-...|X-..-|C-.-.|Y-.--|Z--..|Q--.-|";
	Code_str[4]="0-----|1.----|2..---|3...--|4....-|5.....|6-....|7--...|8---..|9----.|";
	Code_str[5]="..-.-.-|?..--..|!-.-.--|,--..--|";
	Code_str[6]="@-.--.-.";

	Inputpin_int = pin_int; // for analog read
	
	Events_int=(int *)malloc(EVENTS * sizeof(int));
	Times_ulng=(unsigned long *)malloc(EVENTS * sizeof(unsigned long));
	
	// configure the signal input pin
	pinMode(Inputpin_int, INPUT);
	
	Reset();
}

rgMorse::~rgMorse()  {
	free(Events_int);
	free(Times_ulng);
}

/*
** Public interface
*/

void rgMorse::Reset() {
	memset(Events_int, '\0', EVENTS * sizeof(int));
	memset(Times_ulng, '\0', EVENTS * sizeof(unsigned long));
	Eventidx_int=-1;
	Character_char=0;
	computeSignal(-1);
}

boolean rgMorse::Signal() {
	boolean retval_bool=false;
	if (Ambient_int >= 0)
		retval_bool=(Signal_int > Ambient_int + THRESHOLD);
	return retval_bool;
}

int rgMorse::Sigval() {
	return Signal_int;
}

int rgMorse::Ambient() {
	return Ambient_int;
}

char *rgMorse::Morse() {
	return Morse_str;
}

char rgMorse::Character() {
	return Character_char;
}

// take a sample every CLOCKPERIOD ms, else nop
// return value: true=a new character is available; false=computing average in progress
boolean rgMorse::Clock() {
	boolean retval_bool=false;
	static unsigned long last_time_ulng=0;
	unsigned long now_ulng=millis();
	
	// every 10 ms
	if (now_ulng - last_time_ulng >= CLOCKPERIOD) {
		last_time_ulng=now_ulng;
		if (computeSignal(analogRead(Inputpin_int)))
			addEvent(Signal_int, now_ulng);
		if (charComplete()) {
			Character_char=decodeCharacter(); // Returns \0 if character is invalid
			retval_bool=true;
		}
	}
	return retval_bool;
}

/*
** Private implementation
*/

// Returns \0 if character is invalid
char rgMorse::decodeCharacter() {
	char retval_char='\0';
	int bufidx_int=0;
	int buflen_int=10;
	/*
	char debug_str[80];
	sprintf(debug_str, "decodeCharacter() %d events", Eventidx_int + 1);
	Serial.println(debug_str);
	for (int idx_int=0; idx_int <= Eventidx_int; idx_int++) {
		sprintf(debug_str, "idx%d=%d at %ld, ", idx_int, Events_int[idx_int], Times_ulng[idx_int] );
		Serial.print(debug_str);
	}
	Serial.println("");
	*/
	memset(Morse_str, '\0', buflen_int);
	for (int idx_int=1; idx_int <= Eventidx_int && bufidx_int < buflen_int - 1; idx_int++) {
		if (Events_int[idx_int] < Ambient_int + THRESHOLD)
			Morse_str[bufidx_int++] = (Times_ulng[idx_int] - Times_ulng[idx_int - 1] > 3*DITTIME) ? '-' : '.' ;
	}
	
	//sprintf(debug_str, "decodeCharacter() returns %s", Morse_str);
	//Serial.println(debug_str);
	
	retval_char=morse2alpha(Morse_str);

	memset(Events_int, '\0', EVENTS * sizeof(int));
	memset(Times_ulng, '\0', EVENTS * sizeof(unsigned long));
	Eventidx_int=-1;
	return retval_char;
}

// Returns \0 if character is invalid
char rgMorse::morse2alpha(char *morse_str) {
	char retval_char='\0';
	int length_int=strlen(morse_str);
	if (length_int > 0 && length_int <= 7) {
		char key_str[length_int + 2];
		strcpy(key_str, morse_str);
		strcat(key_str, "|");
		char *pos_ptr=strstr(Code_str[length_int-1], key_str);
		if (pos_ptr != NULL)
			retval_char = *(pos_ptr -1); // found
	}
	return retval_char;
}

// test end of character (low since 300 ms or Events_int array full)
boolean  rgMorse::charComplete() {
	boolean retval_bool=false;
	//char debug_str[80];
	if (Eventidx_int > 0) {
		if  (Eventidx_int < EVENTS-1)
			retval_bool=(Events_int[Eventidx_int] < Ambient_int + THRESHOLD && millis() - Times_ulng[Eventidx_int] > 3*DITTIME);
		else
			retval_bool=true; // Events_int array full
	}
	//sprintf(debug_str, "charComplete() returns %s", retval_bool ? "true":"false");
	//Serial.println(debug_str);
	return retval_bool;
}

// this function is called every 50 ms, each time a new averaged signal (computed by computeSignal()) is available
void rgMorse::addEvent(int signal_int, unsigned long time_ulng) {
	//char debug_str[80];
	
	if  (Eventidx_int < EVENTS-1) {
		int last_signal_int=Ambient_int;
		unsigned long last_time_ulng=0;
		int threshold_int=Ambient_int + THRESHOLD;
		if (Eventidx_int >= 0) {
			last_signal_int=Events_int[Eventidx_int];
			last_time_ulng=Times_ulng[Eventidx_int];
		}
		//sprintf(debug_str, "\tsignal: %d/%d, last: %d/%d", signal_int, threshold_int, last_signal_int, threshold_int);
		//Serial.println(debug_str);
		if (((signal_int > threshold_int) && (last_signal_int < threshold_int))
		 || ((signal_int < threshold_int) && (last_signal_int > threshold_int))) {
			if (millis() - last_time_ulng >= DITTIME) { // else ignore events < 100 ms
				Eventidx_int++;
				Events_int[Eventidx_int]=signal_int;
				Times_ulng[Eventidx_int]=time_ulng;
				//sprintf(debug_str, "addEvent(%d, %ld) at idx %d, ambient=%d", signal_int, time_ulng, Eventidx_int, Ambient_int);
				//Serial.println(debug_str);
			}
		}
		if (Eventidx_int == -1) {
			// morse character acquisition not in progress : update Ambient_int
			const int SIGNALS=10; // the signal is averaged over this number of signals
			Ambient_int = (((SIGNALS - 1) * Ambient_int) + signal_int) / SIGNALS;
		}
	} // else Events_int array full
}

// it takes 50 ms (SAMPLES * CLOCKPERIOD) to compute the average value of the signal
// return value: true=a new signal is available (happens every 50 ms); false=computing average in progress
boolean rgMorse::computeSignal(int sample_int) {
	boolean retval_bool=false;
	static const int SAMPLES=5; // the signal is averaged over this number of samples
	static unsigned long signal_sum_ulng=0; // sum of sample values
	static int signal_count_int=0; // number of samples acquired for computing the mean signal value
	
	if (sample_int == -1) {
		// reset
		signal_sum_ulng=0; 
		signal_count_int=0;
		Signal_int=-1;
		Ambient_int=-1;
	}
	else {
		if (signal_count_int< SAMPLES) {
			++signal_count_int;
			signal_sum_ulng += sample_int;
		}
		else {
			Signal_int = signal_sum_ulng / SAMPLES;
			signal_count_int=0;
			signal_sum_ulng=0;
			if (Ambient_int == -1)
				Ambient_int=Signal_int; // use first sample to initialize the ambient value
			retval_bool=true;
		}
	}
	return retval_bool;
}

