#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
#include <rgWiFi.h>
#include "ns_common.h"
#include "ns_gpios.h"
#include "ns_command.h"
#include "ns_network.h"

/* Network definitions *******************************************************************/

static WiFiServer Server_obj(SERVER_PORT);
static WiFiClient Client_obj;

/* Network implementation *******************************************************************/

// /home/rigou/.arduino15/packages/esp32/hardware/esp32/2.0.2/libraries/WiFi/src/WiFiServer.h
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

extern NetInfo NetInfo_struct;

NetInfo NetInfos[6];

void net_Setup() {
	rgTrace(__func__);
	
    NetInfos[0].net_hostname="esp00";

    // connect to my home network
	NetInfos[0].net_ip=IPAddress(192, 168, 2, 30);
	NetInfos[0].net_gateway=GATEWAY_NET2;
	NetInfos[0].net_subnet=SUBNET_NET2;
	NetInfos[0].net_ssid=SSID_NET2;
	NetInfos[0].net_password=PASSWD_NET2;

    // Connect to an ESP32 running as an AP
    // NetInfos[0].net_ip=IPAddress(192, 168, 4, 30);
	// NetInfos[0].net_gateway=GATEWAY_ESP1;
	// NetInfos[0].net_subnet=SUBNET_ESP1;
	// NetInfos[0].net_ssid=SSID_ESP1;
	// NetInfos[0].net_password=PASSWD_ESP1;

#ifdef USE_HW_DEVICE_ID
	NetInfos[1].net_hostname="esp01";
	NetInfos[1].net_ip=IPAddress(192, 168, 2, 31);
	NetInfos[1].net_gateway=GATEWAY_NET2;
	NetInfos[1].net_subnet=SUBNET_NET2;
	NetInfos[1].net_ssid=SSID_NET2;
	NetInfos[1].net_password=PASSWD_NET2;

	NetInfos[2].net_hostname="esp02";
	NetInfos[2].net_ip=IPAddress(192, 168, 1, 111);
	NetInfos[2].net_gateway=GATEWAY_NET1;
	NetInfos[2].net_subnet=SUBNET_NET1;
	NetInfos[2].net_ssid=SSID_NET1;
	NetInfos[2].net_password=PASSWD_NET1;

	NetInfos[3].net_hostname="esp03";
	NetInfos[3].net_ip=IPAddress(192, 168, 2, 33);
	NetInfos[3].net_gateway=GATEWAY_NET2;
	NetInfos[3].net_subnet=SUBNET_NET2;
	NetInfos[3].net_ssid=SSID_NET2;
	NetInfos[3].net_password=PASSWD_NET2;

	NetInfos[4].net_hostname="esp04";
	NetInfos[4].net_ip=IPAddress(192, 168, 2, 34);
	NetInfos[4].net_gateway=GATEWAY_NET2;
	NetInfos[4].net_subnet=SUBNET_NET2;
	NetInfos[4].net_ssid=SSID_NET2;
	NetInfos[4].net_password=PASSWD_NET2;

	NetInfos[5].net_hostname="esp05";
	NetInfos[5].net_ip=IPAddress(192, 168, 2, 35);
	NetInfos[5].net_gateway=GATEWAY_NET2;
	NetInfos[5].net_subnet=SUBNET_NET2;
	NetInfos[5].net_ssid=SSID_NET2;
	NetInfos[5].net_password=PASSWD_NET2;

	// these 3 pins select the device identifier, see net_device_id()
	pinMode(DEVBIT0_GPIO, INPUT_PULLUP);
	pinMode(DEVBIT1_GPIO, INPUT_PULLUP);
	pinMode(DEVBIT2_GPIO, INPUT_PULLUP);
#endif
}

// copy the Netinfo[] item corresponding to the device_id into the NetInfo_struct external var
int net_NetInfo() {
	rgTrace(__func__);
	int retval_int=0;
	int netinfo_idx_int=net_device_id();
	if (netinfo_idx_int < sizeof(NetInfos)/sizeof(NetInfo)) {
		NetInfo_struct=NetInfos[netinfo_idx_int];
		NetInfo_struct.net_dhcp=false;
	}
	else {
		Serial.printf("device id %d out of range\n", netinfo_idx_int);
		retval_int=-1;
	}
	return retval_int;
}

void net_CloseClient() {
	rgTrace(__func__);
	Client_obj.stop();
}

// read data sent by the client, if any (non blocking)
// the caller must call net_CloseClient() after processing the data
// return value: number of characters stored in the buffer, 0=no client connection available or client opened connection but sent no data
int net_Receive(char *buffer_str, const int length_int) {
	rgTrace(__func__);
	//Serial.printf("net_Receive()\n");
	static bool Server_started_bool=false;
	if (!Server_started_bool) {
		Server_obj.begin();
		Server_started_bool=true;
	}
	Client_obj = Server_obj.available();
	int idx_int=0;
	buffer_str[0]='\0';
	if (Client_obj) {
		memset(buffer_str, 0, length_int);
		// wait for the data to become actually readable by Client_obj.read()
		// because sometimes Client_obj.read() returns -1 when called for the first time after boot
		rgwifi_OtaDelay(250);
		while (idx_int<length_int-1) {
			char c1=Client_obj.read(); // returns 255 (-1) if nothing is available
			if (c1 == '\n' || c1 == '\r') {
				if (idx_int==0)
					Serial.printf("DEBUG net_Receive(): empty command\n");
				break;
			}
			else if (c1 == 255) {
				if ((idx_int+1)%3) {
					Serial.printf("DEBUG net_Receive(): invalid command length=%d\n", idx_int+1);
					for (int char_idx_int=0; char_idx_int<=idx_int; char_idx_int++)
						Serial.printf("'%c'(%d) ", buffer_str[char_idx_int], buffer_str[char_idx_int]);
					Serial.printf("\n");
				}
				break;
			}
			else
				buffer_str[idx_int++]=c1;
		}
		Serial.printf("<%s\n", buffer_str);
	}
	//else
	//	Serial.printf("no data available\n");
	return idx_int;
}

void net_Reply(char *reply_str) {
	rgTrace(__func__);
	Serial.printf(">%s\n", reply_str);
    Client_obj.println(reply_str);
}

// gpios DEVBIT0_GPIO, DEVBIT1_GPIO, DEVBIT2_GPIO define the device id
// LOW represents the bit value 1
// return value: a device id in the range [0-7]
#define READ_DEVBIT(x) (digitalRead(x)==LOW?1:0)
static int net_device_id() {
	rgTrace(__func__);
#ifdef USE_HW_DEVICE_ID
	return (READ_DEVBIT(DEVBIT2_GPIO)*4)+(READ_DEVBIT(DEVBIT1_GPIO)*2)+READ_DEVBIT(DEVBIT0_GPIO);
#else
	return 0;
#endif
}
