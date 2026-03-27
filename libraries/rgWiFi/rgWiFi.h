#pragma once

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

// uncomment next line to enable ETA
#define RGWIFI_OTA_ENABLED
#ifdef RGWIFI_OTA_ENABLED
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#endif

// delete this line and set your own network settings in void rgwifi_GetDefaultInfo()
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"

#define WIFILIB_NAME	"rgWiFi"
#define WIFILIB_VERSION	"v1.1.1"

struct NetInfo {
	// required fields
	const char *net_hostname;
	bool net_dhcp; // set it explicitely
	const char *net_ssid;
	const char *net_password;
	// optional fields, set them if net_dhcp is false
	IPAddress net_ip;
	IPAddress net_gateway;
	IPAddress net_subnet;
};

int rgwifi_Reconnect(const char *ssid_str, const char *passwd_str);
int rgwifi_Reconnect(NetInfo connect_info_struct);
int rgwifi_Connect(const char *ssid_str, const char *passwd_str);
int rgwifi_Connect(NetInfo connect_info_struct);
void rgwifi_Disconnect(void);
void rgwifi_GetDefaultInfo(NetInfo *netinfo_out);
void rgwifi_Printinfo(NetInfo info_struct);
void rgwifi_inter_friendly_delay(unsigned long millisec_lng);

#ifdef RGWIFI_OTA_ENABLED
void rgwifi_OtaSetup(const char* hostname_str, const char* ota_password_hash_str);
void rgwifi_OtaDelay(unsigned long millisec_lng);
#else
#define rgwifi_OtaDelay(x)  rgwifi_inter_friendly_delay(x)
#endif
