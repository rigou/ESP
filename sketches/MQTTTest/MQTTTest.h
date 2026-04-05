#pragma once

#define APP_NAME "MQTTTest"		// 12 chars max, see get_info_payload()
#define APP_VERSION "v1.8.1"	// 9 chars max

#ifndef BUILTIN_LED
#define BUILTIN_LED 2
#endif
#define POWERLED_GPIO BUILTIN_LED // this Led shows the power state of the controlled equipment
#define BUTTON_GPIO 33 // gpio of the momentary push button

// comment out next line to disable OTA
#define OTA_ENABLED

// if you enable OTA you must specify the SHA256 hash
// of the password used by espota.py for securing the uploads,
// you obtain it with echo -n "your_passwd" |sha256sum
// #define OTA_PASSWORD_SHA256    "0681db9841c78b52e7daeb49aa05457d9fdb7ae92cba5f22d709377e5bf5247e"

// Set these credentials for your network
// #define WIFI_SSID   "xxxxxx"
// #define WIFI_PASSWD "xxxxxx"
// #define MQTT_HOST   "xxxxxx" // the hostname of the MQTT broker, IP or FQDN ; public.cloud.shiftr.io for testing
// #define MQTT_USER   "xxxxxx"
// #define MQTT_PASSWD "xxxxxx"
// and delete this line :
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
