// source https://www.alejandrowurts.com/projects/esp32-wifi-udp/

#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
#include <WiFi.h>
#include <WiFiUdp.h>

#define TXLED_GPIO 12
#define RUNLED_GPIO 2

WiFiUDP Udp_obj;

char Buffer_str[255];

IPAddress IpClient_obj(192, 168, 4, 30);   // Different IP than server
const unsigned int UdpPort_int = 9999;

void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	delay(500);

	WiFi.softAP(SSID_ESP1, PASSWD_ESP1);
	Udp_obj.begin(UdpPort_int);

	pinMode(RUNLED_GPIO, OUTPUT);
	pinMode(TXLED_GPIO, OUTPUT);

}

void loop() {
	static int count_int=0;
	sprintf(Buffer_str, "packet %d\n", ++count_int);
	Serial.printf("sending %s\n", Buffer_str);
	digitalWrite(TXLED_GPIO, HIGH);
	Udp_obj.beginPacket(IpClient_obj, UdpPort_int);
	Udp_obj.print(Buffer_str);
	Udp_obj.endPacket();
	delay(10);
	digitalWrite(TXLED_GPIO, LOW);
	delay(10);
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
