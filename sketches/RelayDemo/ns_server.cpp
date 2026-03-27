/* ns_server.cpp - a server application framework for ESP32 and ESP8266
 *  Send a command to the server: ./client/espcmd.sh esp00 A00
 *  
 *  OTA is optional, uncomment line "#define RGWIFI_OTA_ENABLED" in library rgWiFi.h to activate it
 *  Perform the OTA update with: OTAupload.sh NetServer 192.168.2.30
 *  
 *  Required libraries: rgNetworkCredentials, rgWiFi
 *  
   2022-04-11 v1.2.1 added command A04 (OTA status)
   2022-04-11 v1.2.2 added commands G,H,L for GPIO control
   2022-04-15 v1.2.3 mod com_Heartbeat() timing, new command prints ip and mac addresses, fixed enable/disable ErrLed
   2022-04-15 v1.2.4 NetServer project moved to RelayDemo, and all its files renamed as ns_*
*/

#include "main.h"

#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
#include <rgWiFi.h>
#include "ns_common.h"
#include "ns_gpios.h"
#include "ns_command.h"
#include "ns_network.h"


// application-global variables are all instantiated here and are refered to with the 'extern' keyword
Feature_struct Features; // structure defined in ns_common.h
NetInfo NetInfo_struct; // structure defined in rgWiFi.h

/*
** MAIN
*/
void ns_setup() {
	rgTrace(__func__);
	Serial.printf("\n\n%s %s\n", APP_NAME, APP_VERSION);
	Serial.printf("%s %s\n", NETSERVER_NAME, NETSERVER_VERSION);
	
	com_RunLed(HIGH);
    com_ErrLed(HIGH);
	net_Setup();
	net_NetInfo();
	//rgwifi_Printinfo(NetInfo_struct);
    while (WiFi.status()!=WL_CONNECTED) {
	    Serial.printf("%s connecting to %s\n", NetInfo_struct.net_hostname, NetInfo_struct.net_ssid);
		rgwifi_Reconnect(NetInfo_struct);
	}
	com_RunLed(LOW);
	com_ErrLed(LOW);

    IPAddress ip_obj=WiFi.localIP();
    String mac_obj=WiFi.macAddress();
    Serial.printf("WiFi connected\n");
	Serial.printf("ip %d.%d.%d.%d, port %d, mac %s\n", ip_obj[0], ip_obj[1], ip_obj[2], ip_obj[3], SERVER_PORT, mac_obj.c_str());
	Serial.printf("send A00 for help on commands\n");

#ifdef RGWIFI_OTA_ENABLED
	rgwifi_OtaSetup(NetInfo_struct.net_hostname, ESPOTA_PASSWORD_HASH);
#endif
}

void ns_loop() {
	rgTrace(__func__);
	if (WiFi.status()==WL_CONNECTED) {
		com_ErrLed(LOW);
		// read a command string from the network (non-blocking) and parse it
		CmdToken cmd_tokens[CMD_TOKENS_MAX];
		int ntokens_int=cmd_Read(cmd_tokens);
		if (ntokens_int>0) {
			// execute each command
			for (int idx_int=0; idx_int<ntokens_int; idx_int++) {
				Command cmd_int=cmd_tokens[idx_int].cmd_int;
				int value_int=cmd_tokens[idx_int].val_int;
				com_RunLed(HIGH);
				if (cmd_Exec(cmd_int, value_int)==0) {
					// wait 1 second after each successfull command
					rgwifi_OtaDelay(500);
					com_RunLed(LOW);
					rgwifi_OtaDelay(500);
				}
				else {
					// wait 1/10 second after each failed command
					rgwifi_OtaDelay(100);
					com_RunLed(LOW);
				}
			}
		}
		net_CloseClient();
	}
	else {
		com_ErrLed(HIGH);
		rgwifi_Reconnect(NetInfo_struct);
	}
}
