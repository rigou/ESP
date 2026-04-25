/* MorseClock.ino
 * 2023-04-21
 */
#include <Arduino.h>
#include <rgCarrier.h>
#include <rgLed.h>
#include <rgMorseEsp.h>
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
#include <rgWiFi.h>
#include <time.h>

#define APP_NAME "MorseClock"
#define APP_VERSION "v1.1.2"
#define DEVICE_ID "esplolin"
#define TIME_SERVER "fr.pool.ntp.org" // "time.nist.gov"

const int PLAYNOW_GPIO = 14; // ground it for continuous time play, labeled D5 on ESP8266

const int SERIAL_BAUDRATE = 115200;
rgMorseEsp Morse_obj;
rgLed Led_obj;

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial)
    ; // wait for serial port to connect
  // give user some time to open the serial monitor
  int count_int = 0;
  Serial.println("");
  while (count_int < 5) {
    Serial.printf("%c", 'A' + count_int++);
    delay(1000);
  }
  Serial.printf("\n%s %s %s\n", APP_NAME, APP_VERSION, TIME_SERVER);
  Serial.printf("\nbuzzer on gpio %d\n", TONE_GPIO);
  pinMode(PLAYNOW_GPIO, INPUT_PULLUP);
}

void print_time(struct tm *timeinfo) {
  Serial.printf("%d-%02d-%02d %02d:%02d:%02d\n", (timeinfo->tm_year) + 1900,
                (timeinfo->tm_mon) + 1, timeinfo->tm_mday, timeinfo->tm_hour,
                timeinfo->tm_min, timeinfo->tm_sec);
}

void play_time(struct tm *timeinfo) {
  char time_str[32];
  int hour12 = (timeinfo->tm_hour > 12 ? timeinfo->tm_hour - 12 : timeinfo->tm_hour);
  sprintf(time_str, "%2d %2d", hour12, timeinfo->tm_min);
  Morse_obj.Play(time_str);
}

// freq_mhz 10,20,40,80,160,240 >=80 for WiFi
void set_cpu_freq(int freq_mhz) {
  Serial.flush();
  Serial.end();
  setCpuFrequencyMhz(freq_mhz);
  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial)
    ;
  Serial.print("CPU Freq = ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");
  Serial.print("XTL Freq = ");
  Serial.print(getXtalFrequencyMhz());
  Serial.println(" MHz");
  Serial.print("APB Freq = ");
  Serial.print(getApbFrequency() / 1000000);
  Serial.println(" MHz");
  Serial.println("");
}

void get_network_time(const char *ntp_server) {
  Led_obj.RunLed(HIGH);
  set_cpu_freq(80); // set normal power mode for WiFi
  while (WiFi.status() != WL_CONNECTED) {
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
  struct tm *timeinfo;
  timeinfo->tm_year = 0;
  time_t time_now;
  while (timeinfo->tm_year <= 70) {
    time_now = time(NULL);
    timeinfo = localtime(&time_now);
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  print_time(localtime(&time_now));

  rgwifi_Disconnect(); // WiFi is no longer needed
  Serial.println("WiFi disconnected");
  set_cpu_freq(10); // set low power mode when WiFi not needed
  Led_obj.RunLed(LOW);
}

void loop() {
  static time_t last_time = 0;
  static time_t Last_time_sync = 0;

  bool playnow_bool = (digitalRead(PLAYNOW_GPIO) == LOW);

  // sync the internal clock
  if (Last_time_sync == 0) {
    get_network_time(TIME_SERVER);
    Last_time_sync = time(NULL); // the current unix timestamp
  }

  time_t time_now = time(NULL);
  if (time_now >= last_time + 1 || playnow_bool) {
	struct tm *timeinfo = localtime(&time_now);
	if (timeinfo->tm_sec == 0 || playnow_bool) {
		print_time(timeinfo);
		play_time(timeinfo);
	} else {
		if (timeinfo->tm_sec > 55) {
			rgcarrier_tone(TONE_GPIO, TONE_FREQ, 5); // 5 ms sound duration sounds like a tick
		}
	}
	Led_obj.RunLed(HIGH);
	delay(20);
	Led_obj.RunLed(LOW);

	playnow_bool = false;
	last_time = time(NULL);
  }

  // resync the internal clock every 24 h
  if (time_now >= Last_time_sync + 86400) {
    get_network_time(TIME_SERVER);
    Last_time_sync = time_now;
  }
}
