#include <Arduino.h> // for delay()
#include "driver/ledc.h"
#include "esp_err.h"
#include "rgCarrier.h"

// call delayMicroseconds() to modulate the carrier

// LEDC constants and structures are defined in ~/.arduino15/packages/esp32/hardware/esp32/2.0.2/tools/sdk/esp32c3/include/hal/include/hal/ledc_types.h
// API documented at https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#
// Actual measurements made with my oscilloscope: precision better than 3/1000 for f=38 kHz
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT

void rgcarrier_stop() {
	ledc_timer_pause(LEDC_MODE, LEDC_TIMER);
	ledc_timer_rst(LEDC_MODE, LEDC_TIMER);
}

void rgcarrier_start() {
	ledc_timer_resume(LEDC_MODE, LEDC_TIMER);
}

// output_gpio_int		output signal to this gpio number
// freq_hz_int			output frequency in Hz, min=4 Hz
// duty_cycle_int		in the range 1-255, 50%=127 ; duty cycle alters the sound volume: 2=minimum, 127=maximum
// invert_output_int		1=gpio output level is LOW by default and pulses are HIGH, 1=gpio output level is HIGH by default and pulses are LOW
void rgcarrier_init(int output_gpio_int, int freq_hz_int, int duty_cycle_int, int invert_output_int) {
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = freq_hz_int,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
	ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = output_gpio_int,
        .duty           = duty_cycle_int,
        .hpoint         = 0,
        .flags.output_invert = invert_output_int // Enable (1) or disable (0) gpio output invert 
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
	rgcarrier_stop();
}

// emulate Arduino's tone() and noTone()
// duration_int	ms, if zero then outputs a continuous tone. Caller can stop it with rgcarrier_noTone().
#define rgcarrier_noTone	rgcarrier_stop
void rgcarrier_tone(int output_gpio_int, int freq_hz_int, int duration_int) {
	rgcarrier_init(output_gpio_int, freq_hz_int, 127, 1);
	rgcarrier_start();
	if (duration_int) {
		delay(duration_int);
		rgcarrier_stop();
	}
}

