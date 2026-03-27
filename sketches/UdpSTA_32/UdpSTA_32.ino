// source https://www.alejandrowurts.com/projects/esp32-wifi-udp/

#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
#include <WiFi.h>
#include <WiFiUdp.h>

#define RXLED_GPIO 13
#define RUNLED_GPIO 2

WiFiUDP Udp_obj;

char Buffer_str[255];

IPAddress IpClient_obj(192, 168, 4, 30);   // Different IP than server
const unsigned int UdpPort_int = 9999;

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	delay(500);

	WiFi.begin(SSID_ESP1, PASSWD_ESP1);
	WiFi.mode(WIFI_STA);
	WiFi.config(IpClient_obj, GATEWAY_ESP1, SUBNET_ESP1);
	Udp_obj.begin(UdpPort_int);

	pinMode(RUNLED_GPIO, OUTPUT);
	pinMode(RXLED_GPIO, OUTPUT);
}

void loop() {
	int packetSize = Udp_obj.parsePacket();
	if (packetSize) {
		digitalWrite(RXLED_GPIO, HIGH);
		int len = Udp_obj.read(Buffer_str, 255);
		if (len > 0)
			Buffer_str[len-1] = 0;
		Serial.printf("received %s\n", Buffer_str);
		delay(10);
		digitalWrite(RXLED_GPIO, LOW);
		delay(10);
	}
	flash_runled();
}

void flash_runled() {
	static int lumi_int=1;
	static int step_int=1;
	const int MAX_int=64;
	if (lumi_int == 0 || lumi_int == MAX_int)
		step_int=-step_int;
	lumi_int+=step_int;
	analogWrite(RUNLED_GPIO, lumi_int);
	delay(8);
}
