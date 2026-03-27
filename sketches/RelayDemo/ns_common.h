#pragma once

#define NETSERVER_NAME	"NetServer" // spaces not permitted
#define NETSERVER_VERSION "v1.2.5"

// uncomment next line to enable function calls tracing when debugging this app
//#define RGNET_TRACE
#ifdef RGNET_TRACE
#define rgTrace(x) 	{ Serial.print("  "); Serial.print(x); Serial.println("()"); }
#else
#define rgTrace(x)
#endif

// if USE_HW_DEVICE_ID is defined below then gpios DEVBIT0_GPIO, DEVBIT1_GPIO, DEVBIT2_GPIO define the device id, see net_device_id()
// else the device id = 0 and DEVBIT0_GPIO, DEVBIT1_GPIO, DEVBIT2_GPIO are ignored
//#define USE_HW_DEVICE_ID 1

// structure instantiated in ns_server.cpp
// update your prefered default values here
struct Feature_struct {
	short Runled_enabled_int=1;
	short Errled_enabled_int=1;
#ifdef RGWIFI_OTA_ENABLED // constant defined in rgWiFi.h
    short Ota_enabled_int=1;
#else
    short Ota_enabled_int=0;
#endif    
};

char *com_Uptime(char *buffer_str);
void com_Heartbeat(int count_int);
void com_RunLed(bool state_bool);
void com_ErrLed(bool state_bool);
