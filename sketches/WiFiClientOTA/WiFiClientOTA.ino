/* WiFiClientOTA.ino  - for ESP32 and ESP8266
 *  This sketch reads a web page using HTTP
 *  2022-04-04 tested OK on ESP32 and ESP8266
 *  2022-12-23 fixed ip ; OTA tested OK on ESP32
 *  2022-12-24 v3.1.0 OTA tested OK on ESP32 and ESP8266 with library rgLed, connecting with fixed ip and with dhcp
 *  2025-03-29 v3.2.1 misc updates, simplification
 * 
 * OTA Upload procedure: see ~/Projects/Espressif/ESP_OTA_usage.txt
 */

// To enable OTA, uncomment "#define RGWIFI_OTA_ENABLED" in rgWiFi.h
#include <rgWiFi.h>

#include <rgLed.h>

#define APP_NAME	"WiFiClientOTA"
#define APP_VERSION	"v3.2.1"

NetInfo NetInfo_struct; // network settings, this type is defined in rgWiFi.h
rgLed Led_obj;

void setup() {
	Serial.begin(115200);
	while (!Serial);
	Serial.print('\n'); for (uint8_t idx=0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); }
	Serial.printf("\n\n%s %s\n", APP_NAME, APP_VERSION);
	Serial.printf("using library %s %s\n", WIFILIB_NAME, WIFILIB_VERSION);
	Serial.printf("using library %s %s\n", LEDLIB_NAME, LEDLIB_VERSION);
	
	// use default network settings for a test device (hostname esp00, fixed ip 192.168.2.30)
	rgwifi_GetDefaultInfo(&NetInfo_struct);
	// NetInfo_struct.net_dhcp=true; // uncomment this line to connect with DHCP instead
	
	// Connect device to WiFi for OTA initialization
	rgwifi_Reconnect(NetInfo_struct);
	if (WiFi.status()==WL_CONNECTED) {
		Serial.println("Initialize OTA");
		rgwifi_OtaSetup(NetInfo_struct.net_hostname, ESPOTA_PASSWORD_HASH);
	}
	else {
		Serial.println("WiFi connection failed");
		while(true);
	}
}

void loop() {
	if (WiFi.status()==WL_CONNECTED) {

		Serial.println("--------------------------------------------------------------------------------");

		ArduinoOTA.handle();
		
		const char* REMOTE_HOSTNAME = "www.google.com";
	    WiFiClient client_obj;
	    Serial.printf("Open TCP connection to %s\n", REMOTE_HOSTNAME);
		if (!client_obj.connect(REMOTE_HOSTNAME, 80)) {
	        Serial.printf("TCP connection to %s failed\n", REMOTE_HOSTNAME);
	        rgwifi_OtaDelay(2000);
	        return;
	    }

		char http_request[100]; // 64 actually used
	    sprintf(http_request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", REMOTE_HOSTNAME);
	    Serial.println("Send HTTP/1.1 Request");
	    client_obj.print(http_request);
	    unsigned long timeout = millis();
	    while (client_obj.available() == 0) {
	        if (millis() - timeout > 5000) {
	            Serial.printf("Client Timeout\n");
	            client_obj.stop();
	            return;
	        }
	    }

		Serial.print("Read contents: ");
	    while(client_obj.available()) {
	        String line = client_obj.readStringUntil('\n');
	        // print the time stored in the Date header
	        if (line.substring(0, 5).equals(String("Date:"))) {
	        	Serial.println(line.substring(6));
	        	break;
	        }
	    }
	    Serial.println("Close TCP connection");
	    client_obj.stop();
	    Led_obj.Heartbeat(NetInfo_struct.net_dhcp?1:2); // 1 flash=dhcp, 2 flashes=fixed ip

	    rgwifi_OtaDelay(10000); // not required because Heartbeat() takes care of this
	}
	else {
		Serial.printf("%s connecting to %s\n", NetInfo_struct.net_hostname, NetInfo_struct.net_ssid);
		rgwifi_Reconnect(NetInfo_struct);
	}
}
