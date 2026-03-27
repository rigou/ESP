/* ns_gpios.h - Hadware configuration
 *	ESP32 recommended gpios: /home/rigou/ELECTRONIQUE/Micro controleurs/ESP32/ESP32 recommended GPIOs.txt
*/

#pragma once

#if ESP32
// ESP-WROOM-32E Development Board
#define RUNLED_GPIO 2		// this activity led provides a visual feedback of the loop() function, see com_RunLed()
#define RUNLED_INVERTED 0	// 0=normal, 1=inverted (on some boards the built-in led logic is inverted)
#endif

#if ESP8266
// ESP8266 NodeMCU dev board
#define RUNLED_GPIO 2		// this led provides a visual feedback of the loop() function, see com_RunLed()
#define RUNLED_INVERTED 1	// 0=normal, 1=inverted (on some boards the built-in led logic is inverted)
#endif

#define ERRLED_GPIO 4		// this red led provides a visual feedback when an error happens
#define ERRLED_INVERTED 0	// 0=normal, 1=inverted (on some boards the built-in led logic is inverted)

// these 3 pins select the device identifier, see net_device_id()
#ifdef USE_HW_DEVICE_ID
#define DEVBIT0_GPIO 13 //gpio of bit0
#define DEVBIT1_GPIO 14 //gpio of bit1
#define DEVBIT2_GPIO 27 //gpio of bit2
#endif
