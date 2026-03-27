/* Blink for ESP32 and ESP8266 - rgLed library test
 *  2022-12-20, 2023-10-25
 *  
 *  LED_BUILTIN GPIO : see rgLed.h
 */

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

#include <rgLed.h>

rgLed RunLed_obj(2); // use LED_BUILTIN
rgLed ErrLed_obj(15);

void setup() {
	Serial.begin(921600);
	while (!Serial) ;
	// give user some time to open the serial monitor
	dbprint('\n'); for (uint8_t idx = 0; idx<12; idx++) { dbprint((char)('A'+idx)); delay(500); }
	dbprintf("\nLED_BUILTIN=%d, RUNLED=%d, ERRLED=%d\n", LED_BUILTIN, RunLed_obj.getGpio(), ErrLed_obj.getGpio());
}

void loop() {
	RunLed_obj.Blink(2000, 50);
	ErrLed_obj.Blink(1000, 50);
}
