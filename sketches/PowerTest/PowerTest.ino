#include <Arduino.h>
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
#include <rgWiFi.h>

#ifndef LED_BUILTIN
	#if ARDUINO_LOLIN32_LITE
		#define LED_BUILTIN 22
	#else
		#define LED_BUILTIN 2
	#endif
#endif

#define APP_NAME	"PowerTest"
#define APP_VERSION	"v1.0.0"
#define DEVICE_ID	"esp00"
const int SERIAL_BAUDRATE=115200;

void setup()
{
	Serial.begin(SERIAL_BAUDRATE);
	while (!Serial) ;
	// give user some time to open the serial monitor
	int count_int=0;
	Serial.println("");
	while (count_int<5) {
		Serial.printf("%c", 'A'+count_int++);
		delay(1000);
	}
	Serial.printf("\n%s %s\n", APP_NAME, APP_VERSION);
	pinMode(LED_BUILTIN, OUTPUT);

	while (WiFi.status()!=WL_CONNECTED) {
	    Serial.printf("%s connecting to %s\n", DEVICE_ID, SSID_NET2);
		rgwifi_Reconnect(SSID_NET2, PASSWD_NET2);
	}
	Serial.print("WiFi connected, IP address ");
	Serial.println(WiFi.localIP());
	// rgwifi_Disconnect();
    // Serial.println("WiFi disconnected");
    
}
 
void loop()
{
	uint32_t freq_int = 0;
	uint32_t new_freq_int = 0; // 10,20,40,80,160,240

	// Board : ESP-WROOM-32E narrow Development Board, powered by its +5V Vin pin
	// Measurement : current flowing through Vin
	//
	// booting 42.8 mA	214 mW
	//
	// running infinite loop, no WiFi :
	// new_freq_int=10;  // 15.02 mA	75 mW
	// new_freq_int=20;  // 17.21 mA
	// new_freq_int=40;  // 21.43 mA
	// new_freq_int=80;  // 38.42 mA
	// new_freq_int=160; // 52.84 mA
	// new_freq_int=240; // 71.40 mA	360 mW

	// running delay(600000), no WiFi, or WiFi disconnected after initial connection :
	// new_freq_int=10;  // 14.23 mA	71 mW
	// new_freq_int=20;  // 15.50 mA
	// new_freq_int=40;  // 18.01 mA
	// new_freq_int=80;  // 30.08 mA
	// new_freq_int=160; // 36.53 mA
	// new_freq_int=240; // 43.16 mA	216 mW
	//
	// running delay(600000) with WiFi connected:
	// new_freq_int=80;  //  35-88 mA fluctuant, peak 118 mA while connecting to AP
	// new_freq_int=160; //  42-85 mA fluctuant, peak 128 mA while connecting to AP
	// new_freq_int=240; //  49-81 mA fluctuant, peak 135 mA while connecting to AP

	if (new_freq_int>0) // else use the freq set in the Arduino IDE
		setCpuFrequencyMhz(new_freq_int);
		
  	Serial.begin(SERIAL_BAUDRATE); // must set serial speed after changing cpu freq
  	
	Serial.println("");
	freq_int = getCpuFrequencyMhz();
	Serial.print("CPU Freq = ");
	Serial.print(freq_int);
	Serial.println(" MHz");
	freq_int = getXtalFrequencyMhz();
	Serial.print("XTL Freq = ");
	Serial.print(freq_int);
	Serial.println(" MHz");
	freq_int = getApbFrequency();
	Serial.print("APB Freq = ");
	Serial.print(freq_int/1000000);
	Serial.println(" MHz");

	// measure current after seing the flashes
	for (int idx=0; idx<5; idx++) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500);
		digitalWrite(LED_BUILTIN, LOW);
		delay(500);
	}
	
	// while (true) // infinite loop
	// 	int idx_int=1;
	
	delay(600000); // 10 minutes
}
