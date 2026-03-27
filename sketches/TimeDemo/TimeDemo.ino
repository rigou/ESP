/* TimeDemo.ino 
 * 2023-04-03
 */
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
#include <rgWiFi.h>
#include <time.h> 

#include "rgCarrier.h"

#define BUZZER_GPIO 16
#define BUZZER_FREQ	1000 // sound freq for cpu speed=240 MHz, lower cpu speed gives lower freq


#define APP_NAME	"TimeDemo"
#define APP_VERSION	"v1.0.0"
#define DEVICE_ID	"esp00"
#define TIME_SERVER	"pool.ntp.org" // "time.nist.gov"

const int LED_PIN=LED_BUILTIN; // notice: inverted logic on ESP8266 and WEMOS LOLIN 32
const int BUZ_PIN=16;
    
void setup() {
	Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	// give user some time to open the serial monitor
	int count_int=0;
	Serial.println("");
	while (count_int<5) {
		Serial.printf("%c", 'A'+count_int++);
		delay(1000);
	}
	Serial.printf("\n%s %s %s\n", APP_NAME, APP_VERSION, TIME_SERVER);
	Serial.printf("\nbuzzer on gpio %d\n", BUZZER_GPIO);
	rgcarrier_init(BUZZER_GPIO, BUZZER_FREQ, 4); // duty cycle alters the sound volume: 2=minimum, 127=maximum
  	pinMode(LED_PIN, OUTPUT);
	get_network_time(TIME_SERVER);
}

// duration : 5 ms sound duration sounds like a tick, 50 ms like a beep
void beep(int duration, int count=1) {
	while (count>0) {
		rgcarrier_start();
		delay(duration);
		rgcarrier_stop();
		--count;
		if (count>0)
			delay(duration);
	}
}

void print_time(struct tm *timeinfo) {
	Serial.printf("%d-%02d-%02d %02d:%02d:%02d\n",(timeinfo->tm_year)+1900,(timeinfo->tm_mon)+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void wait_blink(int count) {
	while (count--) {
		digitalWrite(LED_PIN, LOW);
		delay(25);                     
		digitalWrite(LED_PIN, HIGH); 
		delay(25);                     
	}
}

void get_network_time(const char *ntp_server) {
	while (WiFi.status()!=WL_CONNECTED) {
	    Serial.printf("%s connecting to %s\n", DEVICE_ID, SSID_NET2);
		rgwifi_Reconnect(SSID_NET2, PASSWD_NET2);
	}
	Serial.print("WiFi connected, IP address ");
	Serial.println(WiFi.localIP());

	Serial.printf("Contacting %s", ntp_server);
	const long gmtOffset_sec = 3600;
	const int daylightOffset_sec = 3600;
	configTime(gmtOffset_sec, daylightOffset_sec, ntp_server);
	// wait until time information is available
	struct tm *timeinfo; timeinfo->tm_year=0;
	time_t time_now;
    while (timeinfo->tm_year <= 70) {
    	time_now=time(NULL);
   		timeinfo=localtime(&time_now);
		delay(500);
		Serial.print(".");
    }
    Serial.println("");
    print_time(localtime(&time_now));
	
    rgwifi_Disconnect(); // WiFi is no longer needed
    Serial.println("WiFi disconnected");
}

void loop() {
	static time_t last_time=0;
	time_t time_now=time(NULL); // the current unix timestamp
	if (time_now >= last_time+1) {
		struct tm *timeinfo=localtime(&time_now);
		//print_time(timeinfo);
		if (timeinfo->tm_sec == 0)
			beep(50, 4);
		else if (timeinfo->tm_sec%15==0)
			beep(50, timeinfo->tm_sec/15);
		else
			beep(5);
			
		last_time=time_now;
		wait_blink(1); // 100 ms
	}
}
