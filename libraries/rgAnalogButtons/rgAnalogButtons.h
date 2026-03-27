#pragma once
#include <rgAnalog.h>

#define ABTLIB_NAME     "rgAnalogButtons"
#define ABTLIB_VERSION  "v2.0.1"

class rgAnalogButtons {
	private:
		uint8_t NButtons=0;
		uint16_t *AdcValues;

    public:
		uint8_t BTN_OFF=0;
		rgAnalogButtons(uint8_t nbuttons, uint16_t *adc_values);
		uint8_t get_Button(uint16_t adc_value);
};
