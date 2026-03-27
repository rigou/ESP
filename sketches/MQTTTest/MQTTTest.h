#pragma once

#define APP_NAME "MQTTTest"
#define APP_VERSION "v1.4.2"

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
// #define WIFI_SSID   "xxxxxxxx"
// #define WIFI_PASSWD "xxxxxxxx"
// #define MQTT_HOST   "xxxxxxxx" // the hostname of the MQTT broker, IP or FQDN
// #define MQTT_USER   "xxxxxxxx"
// #define MQTT_PASSWD "xxxxxxxx"
// and delete this line :
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
