/* RelayDemo.ino
    activates a relay (or a led) on pin MY_RELAY_GPIO
    test loop: watch -n 1 ~/ARDUINO/Espressif/RelayDemo/client/espcmd.sh -n esp00 H14L14
** 2022-04-20
*/

#include "main.h"
#include "ns_common.h"
#include "ns_server.h"

// insert your own definitions below
#include <rgWiFi.h>
#define MY_RELAY_GPIO  14
#define OpenRelay(x) digitalWrite(x, LOW)
#define CloseRelay(x) digitalWrite(x, HIGH)

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	delay(500);
    ns_setup();

	Serial.printf("relay gpio %d\n", MY_RELAY_GPIO);
	pinMode(MY_RELAY_GPIO, OUTPUT);
	OpenRelay(MY_RELAY_GPIO);
}

void loop() {
    ns_loop();
	
    // the weaker the signal, the more flashes
    // the tcp/ip connection is reliable up to 7 flashes, unstable at 8 flashes, lost at 9 flashes
    int rssi_int = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi_int);
    com_Heartbeat(abs(rssi_int)/10);
}
