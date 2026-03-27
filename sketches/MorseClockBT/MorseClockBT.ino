/* MorseClockBT.ino
 * 2023-04-22
 */
#include <Arduino.h>
#include <rgCarrier.h>
#include <rgLed.h>
#include <rgMorseEsp.h>
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"
#include <rgWiFi.h>
#include <time.h>

#include "BluetoothSerial.h"

#define APP_NAME "MorseClockBT"
#define APP_VERSION "v1.2.0"
#define DEVICE_ID "esplolin"
#define TIME_SERVER "fr.pool.ntp.org" // "time.nist.gov"

const int SERIAL_BAUDRATE = 115200;
const int MUTE_GPIO = 14; // ground it to mute the clock and play sound only when requested over BT

rgMorseEsp Morse_obj;
rgLed Led_obj;

BluetoothSerial SerialBT;
const int BTRXMAXLEN=10;
bool BTconnected_bool=false;
static bool BTprompt_bool=false; // tell user we are ready to process their commands

void bt_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  const char *text=NULL;
  switch (event) {
    case ESP_SPP_SRV_OPEN_EVT:
      text="BT connected";
      BTconnected_bool=true;
      BTprompt_bool=true;
      break;
    case ESP_SPP_CLOSE_EVT:
      text="BT disconnected";
      BTconnected_bool=false;
      break;
    default:
      text=NULL;
      break;
  }
  if (text)
    Serial.println(text);
}

// read data received over BT until \n is found, and replace this \n with \0
// return value: number of characters stored in buffer_out, not counting the final \0
int bt_read_string(unsigned char buffer_out[]) {
  SerialBT.setTimeout(100); // milliseconds
  int rx_count=SerialBT.readBytesUntil('\n', buffer_out, BTRXMAXLEN-1);
  buffer_out[rx_count]='\0';
  return rx_count;
}

void bt_print_time(struct tm *timeinfo) {
  char buff[20];
  SerialBT.printf("%s\n", sprint_time(buff, timeinfo));
}

// buff_out length=20 allocated by caller
char * sprint_time(char *buff_out, struct tm *timeinfo) {
  sprintf(buff_out, "%4d-%02d-%02d %02d:%02d:%02d", (timeinfo->tm_year) + 1900,
    (timeinfo->tm_mon) + 1, timeinfo->tm_mday, timeinfo->tm_hour,
    timeinfo->tm_min, timeinfo->tm_sec);
  return buff_out;
}

void print_time(struct tm *timeinfo) {
  char buff[20];
  Serial.println(sprint_time(buff, timeinfo));
}

void play_time(struct tm *timeinfo) {
  char time_str[32];
  int hour12 = (timeinfo->tm_hour > 12 ? timeinfo->tm_hour - 12 : timeinfo->tm_hour);
  sprintf(time_str, "%2d %2d", hour12, timeinfo->tm_min);
  Morse_obj.Play(time_str);
}

void get_network_time(const char *ntp_server) {
  Led_obj.RunLed(HIGH);
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
  Led_obj.RunLed(LOW);
}

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
  Serial.printf("buzzer on gpio %d\n", TONE_GPIO);
  Serial.printf("mute on gpio %d\n", MUTE_GPIO);
  pinMode(MUTE_GPIO, INPUT_PULLUP);
  
  SerialBT.register_callback(bt_event_handler);
  SerialBT.begin(DEVICE_ID); //Bluetooth device name
}

void loop() {
	static time_t last_time = 0;
	static time_t Last_time_sync = 0;
	unsigned char BT_buff[BTRXMAXLEN];
  bool mute_bool = (digitalRead(MUTE_GPIO) == LOW);
	bool playnow_bool = false;
  char *bt_command = NULL;

	if (BTconnected_bool) {
		if (BTprompt_bool) {
			SerialBT.print("ready\n");
			BTprompt_bool=false;
		}
		if (SerialBT.available()) {
			int buflen=bt_read_string(BT_buff);
			bt_command=(char *)BT_buff;
			if (strcmp(bt_command, "time") == 0)
				playnow_bool=true;
		}
	}

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
      if (BTconnected_bool)
        bt_print_time(timeinfo);
      if (bt_command || ! mute_bool)
  			play_time(timeinfo);
		} else {
			if (timeinfo->tm_sec > 55 && !mute_bool) {
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
