#pragma once

#define CARLIB_NAME	"rgCarrier"
#define CARLIB_VERSION	"v1.0.0"

#if defined(__cplusplus)
extern "C"
{
#endif
	void rgcarrier_init(int output_gpio_int, int freq_hz_int, int duty_cycle_int, int invert_output_int);
	void rgcarrier_start();
	void rgcarrier_stop();
	void rgcarrier_tone(int output_gpio_int, int freq_hz_int, int duration_int);
#if defined(__cplusplus)
}
#endif
