/*	rgAnalogButtons.cpp handles a Five Direction Navigation Button Module (FDNBM)
	connected to the ESP32 using a single analog input and a resistor ladder
	This module is composed of 5 buttons labeled [UP,DWN,LFT,RHT,MID] sharing a single COM connector
	Buttons are non-latching and only one button can be closed at any given time
	See usage example rgAnalogButtonsTest.ino
*/

/* This program is published under the GNU General Public License. 
 * This program is free software and you can redistribute it and/or modify it under the terms
 * of the GNU General  Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.
 * See the GNU General Public License for more details : https://www.gnu.org/licenses/ *GPL 
*/

#include "rgAnalogButtons.h"

// 0=debug off, 1=output to Serial, 2=output to serial UART1
#define DEBUG_ON 0
// 0=trace off, 1=output to serial, 2=output to serial UART1
#define TRACE_ON 0
#include <rgDebug.h>

// gpio_idx: index in the ADC_GPIOS array of the ADC input
// nbuttons: number of buttons in the module (5)
rgAnalogButtons::rgAnalogButtons(uint8_t nbuttons, uint16_t *adc_values) {
	NButtons=nbuttons;
	AdcValues=adc_values;
}

// Debounce and read current state of the Five Direction Navigation Button Module
// adc_value is the value of the latest ADC conversion, returned by rgAnalog::get_Value() after testing rgAnalog::Available()
// Execution time: 3.1 ms at 80 MHz, higher CPU freq gives same exec time
// return value: 0=OFF, 1="UP", 2="DWN", 3="LFT", 4="RHT", 5="MID"
uint8_t rgAnalogButtons::get_Button(uint16_t adc_value) {
	uint8_t retval=0;
	static uint8_t First_state_id=-1;
	static uint8_t Same_button_count=0;
	uint8_t state_id=NButtons;
	for (int idx=0; idx<NButtons; idx++) {
		if (adc_value < (AdcValues[idx]+AdcValues[idx+1])/2) {
			state_id=idx;
			break;
		}
	}
	if (state_id == First_state_id) {
		// return button id as soon as we get 2 consecutive identical values
		if (++Same_button_count==2)
			retval=state_id;
		else
			retval=NButtons; // return 0 (OFF) : 2 consecutive identical values not reached yet, or same button is continuously pressed
	}
	else {
		// a button has just been pressed,
		// or an already pressed button was released before we get 2 consecutive identical values
		Same_button_count=0;
		First_state_id=state_id;
		retval=NButtons; // return 0 (OFF)
	}
	retval=(retval==NButtons) ? 0 : state_id+1;
	return retval;
}

