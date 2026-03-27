/* rgAnalogButtonsTest.ino - demonstrates how to connect a Five Direction Navigation Button Module (FDNBM)
	to the ESP32 using a single analog input and a resistor ladder
	This module is composed of 5 buttons labeled [UP,DWN,LFT,RHT,MID] sharing a single COM connector
	Buttons are non-latching and only one button can be closed at any given time

	Resistor ladder:
		+3.3V-[R0]-a-[R1]-b-[R2]-c-[R3]-d-[R4]-e-[R5]-f-GND
		R0=1500 R1=330 R2=510 R3=1000 R4=2200 R5=11000
	Connections:
		a=UP button, analog input
		b=DWN button
		c=LFT button
		d=RHT button
		e=MID button
		f=COM button
	Analog input voltages:
		all buttons open:	3.0V
		button UP closed:	0.0V
		button DWN closed:	0.6V
		button LFT closed:	1.2V
		button RHT closed:	1.8V
		button MID closed:	2.4V
*/
#define DEBUG_ON 1
#include <rgDebug.h>
#include <rgAnalog.h>
#include <rgAnalogButtons.h>

#define ADC_INPUTS_COUNT	6
uint8_t ADC_GPIOS[]={36, 39, 34, 35, 32, 33}; // ADC1 tested ok but rgAnalog does not work with ADC2
rgAnalog AnalogReader_obj(ADC_GPIOS, ADC_INPUTS_COUNT, 10, 50);

const uint8_t FDNBM_BUTTONS=5;
const char *FDNBM_NAMES[]={"OFF", "UP", "DWN", "LFT", "RHT", "MID"}; // required: OFF is first
uint16_t FDNBM_VALUES[]={0,150,332,540,715,941}; // required: adc values are in increasing order, OFF is first
rgAnalogButtons Buttons_obj(FDNBM_BUTTONS, FDNBM_VALUES);

void setup() {
	Serial.begin(115200);
	while (!Serial);
	dbprint('\n'); for (uint8_t idx = 0; idx<3; idx++) { dbprint((char)('A'+idx)); delay(500); }
	dbprint('\n');
	dbprintln("Ready");

	AnalogReader_obj.Setup();
}

void loop() {
	if (AnalogReader_obj.ReadLock()) {
		if (AnalogReader_obj.Available()) {
			// check the FDNBM
			uint16_t adc_value=AnalogReader_obj.get_Value(0); // 0 = the FDNBM is attached to the first item of ADC_GPIOS[]
			uint8_t button_id=Buttons_obj.get_Button(adc_value);
			if (button_id)
				dbprintln(FDNBM_NAMES[button_id]);

			// process other analog inputs (potentiometers, sensors...)
			for (int idx=1; idx<ADC_INPUTS_COUNT; idx++) {
				int value=AnalogReader_obj.get_Value(idx);
				//dbprintf("gpio %d: %3u\t", ADC_GPIOS[idx], value);
			}
		}
		AnalogReader_obj.ReadUnlock();
	}
}
