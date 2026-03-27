/* OTA implementation -------------------------------------------------------*/

#include <ArduinoOTA.h>
#include <WiFiUdp.h>

const unsigned int OTA_PORT=3232;

static void inter_friendly_delay(unsigned long millisec_lng);

// Setup the OTA environment
// hostname_str : required by ArduinoOTA.setHostname()
// ota_password_hash_str : the SHA256 hash of the password used by espota.py for securing the uploads,
// you obtain it with echo -n "your_passwd" |sha256sum
// You must open Serial and Wifi before calling this function
void OTASetup(const char* hostname_str, const char* ota_password_hash_str) {
	ArduinoOTA.setPort(OTA_PORT);
	ArduinoOTA.setHostname(hostname_str);
	if (ota_password_hash_str)
		ArduinoOTA.setPasswordHash(ota_password_hash_str);

	ArduinoOTA.onStart([]() {
		const char *type_str;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type_str = "program";
		else // U_SPIFFS
			type_str = "filesystem";
		
		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.printf("updating %s\n", type_str);
	});

	ArduinoOTA.onEnd([]() {
		Serial.println("update complete");
		inter_friendly_delay(1000);
	});

	// ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
	// 	if (millis() - last_ota_time > 500) {
	// 		Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
	// 		last_ota_time = millis();
	// 	}
	// })

	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.printf("Auth Failed\n");
		else if (error == OTA_BEGIN_ERROR) Serial.printf("Begin Failed\n");
		else if (error == OTA_CONNECT_ERROR) Serial.printf("Connect Failed\n");
		else if (error == OTA_RECEIVE_ERROR) Serial.printf("Receive Failed\n");
		else if (error == OTA_END_ERROR) Serial.printf("End Failed\n");
	});
	
	ArduinoOTA.begin();
	Serial.printf("OTA ready, port %d\n", OTA_PORT);
}

// same as delay() but calls ArduinoOTA.handle() every OTA_PERIOD ms
// You must call this function at least once every 5 seconds.
// If you don't need any delay call it with arg millisec_lng=0
void OTADelay(unsigned long millisec_lng) {
	const unsigned int OTA_PERIOD=200; // ms3
	if (millisec_lng) {
		while (millisec_lng>=OTA_PERIOD) {
			ArduinoOTA.handle();
			inter_friendly_delay(OTA_PERIOD);
			millisec_lng-=OTA_PERIOD;
		}
		if (millisec_lng) {
			ArduinoOTA.handle();
			inter_friendly_delay(millisec_lng);
		}
	}
	else
		ArduinoOTA.handle();
}

// same as delay(), blocking but interruption friendly
// other units can't call this function
static void inter_friendly_delay(unsigned long millisec_lng) {
	unsigned long start_millis=millis();
	while (millis()-start_millis < millisec_lng)
		yield(); // required for ESP8266, NOP for other SOCs
}
