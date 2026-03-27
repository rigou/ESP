/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-set-custom-hostname-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <WiFi.h>
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

#ifndef BUILTIN_LED
#define BUILTIN_LED 2
#endif

// Change the hostname

void initWiFi() {
	dbprint("Connecting to WiFi...");
	WiFi.begin(WIFI_SSID, WIFI_PASSWD);
	while (WiFi.status() != WL_CONNECTED) {
		dbprint('.');
		delay(1000);
	}
	dbprint('\n');

	byte mac_address[6];
	char hostname[11];
	WiFi.macAddress(mac_address);
	sprintf(hostname, "esp-%02x%02x%02x", mac_address[3], mac_address[4], mac_address[5]);
	WiFi.setHostname(hostname);

	dbprint("ESP32 HostName: ");
	dbprintln(WiFi.getHostname());
	dbprint("ESP32 IP Address: ");
	dbprintln(WiFi.localIP());
	dbprint("ESP32 MAC Address: ");
	dbprintln(WiFi.macAddress());
}

void setup() {
	Serial.begin(921600);
    while (!Serial);
#if DEBUG_ON
	dbprint('\n'); for (uint8_t idx=0; idx<13; idx++) { dbprint((char)('A'+idx)); delay(500); }
#endif
	dbprint('\n');
	pinMode(BUILTIN_LED, OUTPUT);

	initWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(BUILTIN_LED, LOW);
  delay(900);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);
}

