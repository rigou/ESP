#include <rgWiFi.h>
#include "ns_common.h"
#include "ns_gpios.h"

extern Feature_struct Features; // structure instantiated in ns_server.cpp

// return uptime in given 11 characters buffer
char *com_Uptime(char *buffer_str) {
	rgTrace(__func__);
	unsigned long now_lng=millis()/1000;
	int h_int=now_lng/3600;
	int m_int=(now_lng-(h_int*3600))/60;
	int s_int=now_lng-(h_int*3600)-(m_int*60);
	sprintf(buffer_str, "%02d:%02d:%02d", h_int, m_int, s_int);
	return buffer_str;
}

// provides a visual feedback of the loop() function
// does not use delay() to allow interruptions
// duration: count_int = 1:1s, 2:2s, 3:2.5s, 4:3s, 5:3.5s
void com_Heartbeat(int count_int) {
	rgTrace(__func__);
	static unsigned long Last_time=0;
	unsigned long now_lng=millis();
	for (byte idx_int=0; idx_int<count_int; idx_int++) {
		unsigned int pulse_len_int=0;
		
		com_RunLed(HIGH);
		rgwifi_OtaDelay(50); // ms
		com_RunLed(LOW);
		rgwifi_OtaDelay(450); // ms
	}
	rgwifi_OtaDelay(count_int==1?500:1000); // delay between each sequence of flashes
}

void com_RunLed(bool state_bool) {
	//rgTrace(__func__);
	if (Features.Runled_enabled_int) {
		static short Run_led_init_int=0;
		if (!Run_led_init_int) {
			pinMode(RUNLED_GPIO, OUTPUT);
			Run_led_init_int=1;
		}
		#if RUNLED_INVERTED == 1
			digitalWrite(RUNLED_GPIO, !state_bool);
		#else
			digitalWrite(RUNLED_GPIO, state_bool);
		#endif
	}
}

void com_ErrLed(bool state_bool) {
	//rgTrace(__func__);
	if (Features.Errled_enabled_int) {
		static short Err_led_init_int=0;
		if (!Err_led_init_int) {
			pinMode(ERRLED_GPIO, OUTPUT);
			Err_led_init_int=1;
		}
		#if ERRLED_INVERTED == 1
			digitalWrite(ERRLED_GPIO, !state_bool);
		#else
			digitalWrite(ERRLED_GPIO, state_bool);
		#endif
	}
}


#if 0
// return the LSB binary string representation of given integer value, with given number of bits, into given caller-allocated buffer
char *com_BinStr(char *buffer_str, int nbits_int, unsigned int num_int) {
	unsigned int idx_int=0;
	unsigned int mask_int=1;
    while (idx_int < nbits_int) {
    	buffer_str[idx_int]=(mask_int & num_int)?'1':'0';
        mask_int<<=1;
        idx_int++;
    }
	buffer_str[idx_int]='\0';
    return buffer_str;
}
#endif
