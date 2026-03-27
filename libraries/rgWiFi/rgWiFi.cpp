// rgWiFi.cpp - WiFi connection with optional OTA for ESP32 and ESP8266
// see type IPAddress in ~/.arduino15/packages/esp32/hardware/esp32/2.0.2/cores/esp32/IPAddress.h
// and ~/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/cores/esp8266/IPAddress.h
// 2022-04-09
// 2022-12-21 replaced delay() by rgwifi_inter_friendly_delay()
// 2022-12-23 rgwifi_Connect() bugfix: added dns1, dns2 args in WiFi.config() call see https://github.com/esp8266/Arduino/issues/6957)
// 2023-04-16 rgwifi_Disconnect() powers off ADC
// 2025-03-16 removed adc power saving code because the legacy adc driver is deprecated

#include "rgWiFi.h"

// uncomment next line to enable function calls tracing when debugging this library
//#define RGWIFI_TRACE
#ifdef RGWIFI_TRACE
#define rgTrace(x) 	{ Serial.print("  "); Serial.print(x); Serial.println("()"); }
#else
#define rgTrace(x)
#endif

// Customize this function for your own network and delete #include ".../NetworkCredentials.h" in rgWiFi.h
// store default settings for a fixed-ip test device into given user-allocated NetInfo structure
void rgwifi_GetDefaultInfo(NetInfo *netinfo_out) {
	netinfo_out->net_hostname="esp00";
	netinfo_out->net_ssid=WIFI_SSID;
	netinfo_out->net_password=WIFI_PASSWD;
	netinfo_out->net_dhcp=false;
	netinfo_out->net_ip=IPAddress(192, 168, 2, 30);	// ignored if USE_DHCP
	netinfo_out->net_gateway=WIFI_GATEWAY;			// ignored if USE_DHCP
	netinfo_out->net_subnet=WIFI_SUBNET;			// ignored if USE_DHCP
}

/* WiFi implementation ------------------------------------------------------
** Nothing to customize after this line
*/

// WL_ error values defined in ~/.arduino15/packages/esp32/hardware/esp32/2.0.2/libraries/WiFi/src/WiFiServer.h
//typedef enum {
//    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
//    WL_IDLE_STATUS      = 0,
//    WL_NO_SSID_AVAIL    = 1,
//    WL_SCAN_COMPLETED   = 2,
//    WL_CONNECTED        = 3,
//    WL_CONNECT_FAILED   = 4,
//    WL_CONNECTION_LOST  = 5,
//    WL_DISCONNECTED     = 6
//} wl_status_t;

// check if WiFi is connected and (re)connect to the network corresponding to given SSID using DHCP
// call this function in your main loop()
// return value: WL_CONNECTED on success or another WL_ value on error
int rgwifi_Reconnect(const char *ssid_str, const char *passwd_str) {
    rgTrace(__func__);
	if (WiFi.status()==WL_CONNECTED) return WL_CONNECTED;
	
	NetInfo connect_info_struct;
	connect_info_struct.net_dhcp=true;
	connect_info_struct.net_ip=INADDR_NONE;
	connect_info_struct.net_ssid=ssid_str; 
	connect_info_struct.net_password=passwd_str;
	return rgwifi_Reconnect(connect_info_struct);
}

// check if WiFi is connected and (re)connect to the network specified by given NetInfo structure
// call this function in your main loop()
// return value: WL_CONNECTED on success or another WL_ value on error
int rgwifi_Reconnect(NetInfo connect_info_struct) {
    rgTrace(__func__);
	if (WiFi.status()==WL_CONNECTED) return WL_CONNECTED;
	
	// increase this counter after each connection error, reset it on success
	static short int Connection_errors_int=0;
	const short int REBOOT_AFTER_MAX_ERRORS=5;
	
	int status_int=rgwifi_Connect(connect_info_struct);
	if (status_int != WL_CONNECTED) {
		Connection_errors_int++;
		Serial.printf("WiFi connection failed %d/%d : status %d\n", Connection_errors_int, REBOOT_AFTER_MAX_ERRORS, status_int);
		if (Connection_errors_int==REBOOT_AFTER_MAX_ERRORS) {
			Serial.printf("WiFi connection failed %d times, reboot\n", Connection_errors_int);
			rgwifi_inter_friendly_delay(5000);
		    ESP.restart();
		}
		// wait before returning
		for (int count_int=0; count_int<5; count_int++) {
			Serial.printf("waiting %d\n", count_int+1);
			rgwifi_inter_friendly_delay(1000);
		}
	}
	if (status_int == WL_CONNECTED)
		Connection_errors_int=0; // success
		
	return WiFi.status();
}

// connect to the network corresponding to given SSID using DHCP
// return value: WL_CONNECTED on success or another WL_ value on error
int rgwifi_Connect(const char *ssid_str, const char *passwd_str) {
    rgTrace(__func__);
	if (WiFi.status()==WL_CONNECTED) return WL_CONNECTED;
	
	NetInfo connect_info_struct;
	connect_info_struct.net_dhcp=true;
	connect_info_struct.net_ip=INADDR_NONE;
	connect_info_struct.net_ssid=ssid_str; 
	connect_info_struct.net_password=passwd_str;
	return rgwifi_Connect(connect_info_struct);
}

// connect to the network specified by given NetInfo structure
// return value: WL_CONNECTED on success or another WL_ value on error
int rgwifi_Connect(NetInfo connect_info_struct) {
    rgTrace(__func__);
    if (WiFi.status()==WL_CONNECTED) return WL_CONNECTED;
	
	int timeout_int=10000; // ms, must be multiple of 100
	WiFi.mode(WIFI_STA); // not required but good practice
    if (! connect_info_struct.net_dhcp) {
		// connect using given IP address
		WiFi.config(
			connect_info_struct.net_ip, 
			connect_info_struct.net_gateway, 
			connect_info_struct.net_subnet, 
			connect_info_struct.net_gateway,	// dns1
			connect_info_struct.net_gateway		// dns2
		);
    }
    WiFi.begin(connect_info_struct.net_ssid, connect_info_struct.net_password);
    
    while (WiFi.status() != WL_CONNECTED && timeout_int>0) {
        rgwifi_inter_friendly_delay(100);
    	timeout_int-=100;
    }
    
    /* if (WiFi.status()==WL_CONNECTED)
		Serial.printf("debug rgwifi_Connect() connected\n");
	else
		Serial.printf("debug rgwifi_Connect() error %d\n",WiFi.status());
    */
    return WiFi.status();
}

void rgwifi_Disconnect(void) {
    rgTrace(__func__);
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
}

void rgwifi_Printinfo(NetInfo info_struct) {
    rgTrace(__func__);
	Serial.print("net_hostname="); 	Serial.println(info_struct.net_hostname);
	Serial.print("net_dhcp="); 		Serial.println(info_struct.net_dhcp);
	Serial.print("net_ssid="); 		Serial.println(info_struct.net_ssid);
	Serial.print("net_password="); 	Serial.println(info_struct.net_password);
	Serial.print("net_ip="); 		Serial.print(info_struct.net_ip);
	Serial.print(" ("); Serial.print(info_struct.net_dhcp?"dhcp":"fixed ip"); Serial.println(")");
	Serial.print("net_gateway="); 	Serial.println(info_struct.net_gateway);
	Serial.print("net_subnet="); 	Serial.println(info_struct.net_subnet);
}

// same as delay(), blocking but interruption friendly
void rgwifi_inter_friendly_delay(unsigned long millisec_lng) {
	unsigned long start_millis=millis();
	while (millis()-start_millis < millisec_lng)
		yield(); // required for ESP8266, does NOP for other SOCs
}

/* OTA implementation -------------------------------------------------------*/
#ifdef RGWIFI_OTA_ENABLED
const unsigned int OTA_PORT=3232;

// Setup the OTA environment
// hostname_str : required by ArduinoOTA.setHostname()
// ota_password_hash_str : the OTA password hash for securing uploads with espota.py, or NULL if no security
// requires prior opening of Serial and Wifi connection by the calling code
void rgwifi_OtaSetup(const char* hostname_str, const char* ota_password_hash_str) {
    rgTrace(__func__);
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
        rgwifi_inter_friendly_delay(1000);
	});
	//ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		//Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	//});
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
// notice: ensure your app calls ArduinoOTA.handle() at least once every 5 seconds
// either directly or indirectly by calling this function
void rgwifi_OtaDelay(unsigned long millisec_lng) {
	//rgTrace(__func__);
	const unsigned int OTA_PERIOD=200; // ms
	while (millisec_lng>=OTA_PERIOD) {
		ArduinoOTA.handle();
		rgwifi_inter_friendly_delay(OTA_PERIOD);
		millisec_lng-=OTA_PERIOD;
	}
	if (millisec_lng) {
		ArduinoOTA.handle();
		rgwifi_inter_friendly_delay(millisec_lng);
	}
}

#endif
