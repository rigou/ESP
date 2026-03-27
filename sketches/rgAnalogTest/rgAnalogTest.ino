#define DEBUG_ON 1
#include <rgDebug.h>
#include <rgAnalog.h>

#define ADC_INPUTS_COUNT	6
uint8_t ADC_GPIOS[]={36, 39, 34, 35, 32, 33}; // ADC1 tested ok
//uint8_t ADC_GPIOS[]={27, 13, 14, 25, 26, 12}; // ADC2 does not work
rgAnalog AnalogReader_obj(ADC_GPIOS, ADC_INPUTS_COUNT, 10, 50);

void setup() {
	Serial.begin(115200);
	while (!Serial);
	dbprint('\n'); for (uint8_t idx = 0; idx<3; idx++) { dbprint((char)('A'+idx)); delay(500); }
	dbprint('\n');
	dbprintln("Ready");

	AnalogReader_obj.Setup();
}

void loop() {
	static unsigned long Count=0;
	static unsigned long Last_count=0;
	static unsigned long Clock_timer=millis();
	
	unsigned long call_timer=micros();
	if (AnalogReader_obj.ReadLock()) {
		if (AnalogReader_obj.Available()) {
			++Count;
			dbprintf("%4lu\t", Count);
			for (int idx=0; idx<ADC_INPUTS_COUNT; idx++) {
				int value=AnalogReader_obj.get_Value(idx);
				//dbprintf("gpio %d: %3u\t", ADC_GPIOS[idx], value);
			}
			//dbprint('\n');
			unsigned long call_duration=micros()-call_timer;
			dbprintf("call %lu microsec\n", call_duration); // 83 microsec (ADC_INPUTS_COUNT=6), 79 microsec (ADC_INPUTS_COUNT=1)
		}
		AnalogReader_obj.ReadUnlock();
	}
	delay(9);

	if (millis()>Clock_timer+1000) {
		dbprintf("%lu conv/sec\n", Count-Last_count); // 55 conv/sec (ADC_INPUTS_COUNT=6), 112 conv/sec (ADC_INPUTS_COUNT=1)
		Clock_timer=millis();
		Last_count=Count;
	}

}
