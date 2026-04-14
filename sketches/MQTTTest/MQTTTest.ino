// MQTTTest.ino - MQTT client example example
// this program uses the arduino-mqtt library by Joël Gähwiler, which is installed in Projects/ESP/libraries/MQTT
// (install it with the Arduino IDE Library manager)
// https://github.com/256dpi/arduino-mqtt
// 2026-04-03

/* Best Practices for Designing MQTT Message Topics
    1) Omit Leading Forward Slash in Topics: Avoid using a leading forward slash
        in MQTT topics as it adds an unnecessary top-level topic and can cause confusion.
    2) No Spaces in MQTT Topics: Spaces can complicate readability and debugging. 
        Use continuous strings without spaces or unusual characters to enhance clarity.
    3) Short and Concise Topics: Keep MQTT topics brief to reduce network load 
        and resource usage, vital for devices with limited resources.
    4) Use ASCII Characters Only: Stick to ASCII characters in MQTT topics for consistency
        and to avoid issues with non-ASCII characters that may display incorrectly.
    5) Include Unique Identifiers in Topics: Embed a unique identifier or client ID
        in the topic for better message tracking and control over publishing permissions.
*/

/*  Build commands:
        cd $HOME/Projects/ESP/sketches
        compile and upload via USB:	$HOME/Projects/ESP/sketches/bwacli.sh MQTTTest -u ttyUSB0

    Test commands:
    	terminal #1: mosquitto_sub -h nodex.local -u demo -P demo -v -t esp04/Button/get

	    terminal #2:
        turn on POWERLED_GPIO:
            mosquitto_pub -h nodex.local -u demo -P demo -t esp04/Power/set -m 1
        turn off POWERLED_GPIO:
            mosquitto_pub -h nodex.local -u demo -P demo -t esp04/Power/set -m 0
*/

#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

#include "MQTTTest.h"

#ifdef OTA_ENABLED
#include <rgOTA.h>
#define InterruptDelay OTADelay // call OTA.OTADelay() instead of InterruptDelay()
bool Ota_initialized=false;
#endif

#include <rgParam.h>
rgParam FPar_obj;

#include <Pushbutton.h> // Debounce library by Pololu, install it with the IDE Tools/Manage libraries
Pushbutton Button_obj(BUTTON_GPIO);

WiFiClient WifiClient_obj;

// The maximum size for packets being published and received is set by default to 128 bytes. 
// The receiving of messages is constrained by the read buffer, which may be increased if necessary.
// To change the read buffer size, you need to use MQTTClient MqttClient_obj(256) or MQTTClient MqttClient_obj(256, 128)
// (128 bytes is enough for the write buffer because the message payload is sent directly during publishing)
MQTTClient MqttClient_obj(128);

// Full topic format is like "client_id/topic/get" or "client_id/topic/set". No leading slash.
const int FULLTOPIC_LEN=30;
// The max payload length is 76 in this example, see get_info_payload()
const int PAYLOAD_LEN=76;

// contents of the latest received message, set by onMsg()
// if the next message arrives before the processing of this data is complete
// then this data will be overwritten by the next message,
// Solution: onMsg() should store the data in a queue of Mqtt_Message structures
static struct Mqtt_Message {
	bool available=false;           // a new message is available
	char fulltopic[FULLTOPIC_LEN+1];// a null-terminated c-string
	char payload[PAYLOAD_LEN+1];    // a null-terminated c-string
} Msg;

// Topics are named from the MQTT broker's perspective, their names are case-sensitive
// Topics names are made of 3 words separated by 2 '/' : prefix / topic_name / suffix
//      MqttClid is the prefix, and the suffix is "get" or "set"
// * we subscribe to the "set" topics, and we execute the corresponding commands upon receiving a message from Home Assistant (HA)
// * we publish to the "get" topics when our internal state changes, HA subscribes to these topics and processes them
// In this example, the following topics control the state of a fictional equipment controlled by the ESP32 device :
// Subscribed topics - function connect() makes the subscriptions:
//      "MqttClid/Power/set"			HA turns power on/off ('1'=on, '0'=off) of the controlled equipment
//      "MqttClid/Temp/set" 	        HA sets the temperature on the thermostat of the controlled equipment
// Published topics:
//      "MqttClid/Online/get"			HA reads our birth message/last will testament during MqttClient_obj.connect() (1=online, 0=offline)
//      "MqttClid/Power/get"			HA reads power state of the controlled equipment
//      "MqttClid/Temp/get" 	        HA reads the temperature set on the thermostat of the controlled equipment
//      "MqttClid/Button/get"           HA reads the push button ('1'=closed, '0'=open)
//      "MqttClid/Info/get"			    HA reads our status and misc information

// Device state storage
struct DeviceState {
    // MQTT-related
	unsigned int Power=0; 	// 0=off, 1=on
	unsigned int Temp=0;
	unsigned int Button=0;
    bool Changed=false; // true=update settings file

    // Non-MQTT related
    char Hostname[11]; // 10 chars + \0
    char MqttClid[11]; // 10 chars + \0
    IPAddress LocalIp=INADDR_NONE;
} State;
    
void setup() {
    Serial.begin(921600);
    while (!Serial);
#if DEBUG_ON
	dbprint('\n'); for (uint8_t idx=0; idx<13; idx++) { dbprint((char)('A'+idx)); delay(500); }
#endif
	dbprintf("\n\n%s %s\n", APP_NAME, APP_VERSION);
	
    pinMode(POWERLED_GPIO, OUTPUT);
	digitalWrite(POWERLED_GPIO, LOW);
	pinMode(BUTTON_GPIO, INPUT_PULLUP);

    // Read the latest settings from file
    // Init() expects these args: format_fs_if_failed, max_len_path, max_len_key, max_len_value, max_records
    const char SETTINGS_FILE[]="settings.txt"; // move to setup()
    if (FPar_obj.Init(SETTINGS_FILE, 1, 12, 8, 8, 3) == 0) {
        FPar_obj.SetSeparator('=');
        if (FPar_obj.Load() == -1) {
            dbprintln("settings file not found, write default values");
            FPar_obj.SetKeyInt("Power", 0);
            FPar_obj.SetKeyInt("Temp", 20);
            FPar_obj.SetKeyInt("Button", 0);
            if (FPar_obj.Save() < 0) {
                EndProgram("write settings file error", false);
            }
        }
        if (FPar_obj.Load() < 0) {
            EndProgram("read settings file error", false);
        }
        FPar_obj.GetKeyInt("Power", &State.Power);
        FPar_obj.GetKeyInt("Temp", &State.Temp);
        FPar_obj.GetKeyInt("Button", &State.Button);
    }
    else {
        EndProgram("open settings file error", false);
    }
    
    // initialize the device hardware with the values read in the settings file
    // in this example we just need to set the power of the controlled equipment
    // but in a more realistic implementation we should also set the thermostat of the controlled equipment
    digitalWrite(POWERLED_GPIO, State.Power); 

    MqttClient_obj.onMessageAdvanced(onMsg);
    MqttClient_obj.begin(MQTT_HOST, WifiClient_obj); // public.cloud.shiftr.io for testing
}

void loop() {
    bool connected = WiFi.status() == WL_CONNECTED && MqttClient_obj.connected();

    if (!connected) {
        connected=connect();
        if (connected) {
            publish_device_state();
#ifdef OTA_ENABLED
            if (!Ota_initialized) { // no need to redo ota initialization after reconnecting wifi
                OTASetup(State.Hostname, OTA_PASSWORD_SHA256);
                dbprintln("OTA initialized");
                Ota_initialized=true;
            }
#endif
        }
    }
    if (connected) {
        // check if the momentary push button has been pressed and released
		if (Button_obj.getSingleDebouncedRelease()) {
			State.Button = !State.Button; // toggle the button state
            State.Changed=true;
			publish("Button", State.Button);
		}

        MqttClient_obj.loop();

        if (Msg.available) {
            process_message();
            Msg.available=false;
        }

        // persist the device state into the settings file
        // update the settings file not faster than once every 10 seconds (Flash memory life = 10.000 writes)
        static unsigned long Last_file_update=0;
        if (State.Changed && millis()-Last_file_update>=10000) {
            FPar_obj.SetKeyInt("Power", State.Power);
            FPar_obj.SetKeyInt("Temp", State.Temp);
            FPar_obj.SetKeyInt("Button", State.Button);
            if (FPar_obj.Save() < 0) {
                EndProgram("write settings file error", false);
            }
            State.Changed=false;
            Last_file_update=millis();
        }
        // if OTA_ENABLED then OTA.OTADelay() will be called instead, 
        // and it will call ArduinoOTA.handle(), which is required for OTA.
        // * make sure to call this function periodically in function loop(), 
        // * if you don't need any delay in your loop() call it anyway with  millisec_lng = 0 
        InterruptDelay(0);
    }
    else
        InterruptDelay(5000);
}

void subscribe_all(void) {
    // add subscriptions here
    subscribe("Power");
    subscribe("Temp");         
}

void process_message(void) {
    dbprintf("process_message %s : \"%s\"\n", Msg.fulltopic, Msg.payload);
    const char *topic_name = nullptr;
    const char *topic_suffix = nullptr;
    if (parse_topic(&topic_name, &topic_suffix)) {
        if (strcmp(topic_name, "Power") == 0) {
            if (strcmp(topic_suffix, "set") == 0) {
                State.Power = atoi(Msg.payload);
                State.Changed = true;
                publish("Power", State.Power);
                // set the power of the controlled equipment accordingly
                digitalWrite(POWERLED_GPIO, State.Power);
            }
        }
        else if (strcmp(topic_name, "Temp") == 0) {
            if (strcmp(topic_suffix, "set") == 0) {
                State.Temp = atoi(Msg.payload);
                State.Changed = true;
                publish("Temp", State.Temp);
                // set the temperature of the thermostat on the controlled equipment accordingly (not implemented in this example)
            }
        }
     }
    else {
        dbprintf("process_message() failed: invalid topic %s\n", Msg.fulltopic);
    }
}

bool publish_device_state(void) {
	trprintln(__func__);
	bool retval=false;
	retval  = publish("Power", State.Power);
	retval &= publish("Temp", State.Temp);
	retval &= publish("Button", State.Button);
	retval &= publish("Info", get_info_payload());
	return retval;
}

// compose a json digest of the device state
// {
//   "app_name": "MQTTTest",
//   "app_version": "v1.4.0",
//   "ip": "192.168.2.164"
// }
char *get_info_payload(void) {
	trprintln(__func__);
	static char json_buffer[PAYLOAD_LEN+1];
	snprintf(json_buffer, PAYLOAD_LEN,
		"{\"app_name\":\"%s\",\"app_version\":\"%s\",\"ip\":\"%s\"}",
		APP_NAME, APP_VERSION, State.LocalIp.toString().c_str()
	);
	return json_buffer; // 76 characters max : {"app_name":"123456789abc","app_version":"v12.34.56","ip":"192.168.111.222"}
}

// DO NOT CHANGE ANYTHING AFTER THIS LINE -------------------------------------

static char Full_Topic_Buffer[FULLTOPIC_LEN+1]; // used by publish() and subscribe()

// Compute the full name of given topic into Full_Topic_Buffer[]
char *get_full_topic_name(const char* topic_name, const char *suffix) {
    snprintf(Full_Topic_Buffer, FULLTOPIC_LEN+1, "%s/%s/%s", State.MqttClid, topic_name, suffix);
    return Full_Topic_Buffer;
}

// Connect to WiFi if not already done, connect to the MQTT broker and subscribe to topics
// Return value: true=fully connected to WiFi and MQTT broker
bool connect() {
    bool retval=false;
    if (WiFi.status() != WL_CONNECTED) {
        dbprintf("connecting to WiFi %s\n", WIFI_SSID);
        WiFi.begin(WIFI_SSID, WIFI_PASSWD);
        // it takes up to 2s to have a valid WiFi.status()
        WiFi.waitForConnectResult(5000); 
    }
    if (WiFi.status() == WL_CONNECTED) {
        // generate a unique hostname from the wifi adapter mac address
        // A hostname can only include ASCII letters (a-z, A-Z), digits (0-9), and hyphens (-).
        // It must not start or end with a hyphen and cannot contain spaces or special characters like underscores
        byte mac_address[6];
        WiFi.macAddress(mac_address);
        sprintf(State.Hostname, "esp-%02x%02x%02x", mac_address[3], mac_address[4], mac_address[5]);
        WiFi.setHostname(State.Hostname);

        // use the hostname as the client ID for MQTT
        // this is not mandatory, you could use any other unique name
        strncpy(State.MqttClid, State.Hostname, sizeof(State.MqttClid));

        State.LocalIp=WiFi.localIP();
        dbprintf("%s connected %s\n", State.Hostname, State.LocalIp.toString().c_str());

        if (!MqttClient_obj.connected()) {
            // set our will message (last testament) that gets registered on the broker after connecting
            // if broker connection is lost, broker will auto-publish this message after 10 seconds
            // and Home Assistant will be notified (see device configuration file config_mqtt_dev_esp-dafde8.yaml)
            MqttClient_obj.setWill(get_full_topic_name("Online", "get"), "0", false, 1); // "0"=offline, false=do not retain, qos=1
            // connect to the MQTT broker
            if (MqttClient_obj.connect(State.MqttClid, MQTT_USER, MQTT_PASSWD)) {
                dbprintf("%s connected to MQTT broker %s with client ID %s\n", State.Hostname, MQTT_HOST, State.MqttClid);
                publish("Online", "1"); // tell Home Assistant we are online now
                subscribe_all();
                retval=true;
            }
            else
                dbprintf("connection to MQTT broker %s failed\n", MQTT_HOST);
        }
    }
    else
        dbprintf("connection to WiFi SSID %s failed\n", WIFI_SSID);

    return retval;
}

// publish given payload c-string to given topic on the mqtt broker
// topic_name is the name of the topic, eg "Power", "Switch" ...
// payload is a null terminated c-string
// publish with QoS 1 (guaranteed delivery but may have duplicates), and retain message
bool publish(const char* topic_name, const char* payload) {
    bool retval=false;
    get_full_topic_name(topic_name, "get");
    dbprintf("publish(%s) : %s \"%s\"\n", topic_name, Full_Topic_Buffer, payload);
    retval = MqttClient_obj.publish(Full_Topic_Buffer, payload, true, 1); // retain=true, qos=1
    if (!retval)
        dbprintf("publish(%s) failed\n", topic_name);
	return retval;
}

// overload of above method for integer payloads
bool publish(const char* topic_name, int payload) {
    char payload_str[12];
    snprintf(payload_str, sizeof(payload_str), "%d", payload);
    return publish(topic_name, payload_str);
}

bool subscribe(const char *topic_name) {
    bool retval=false;
    get_full_topic_name(topic_name, "set");
    dbprintf("subscribe(%s) : %s\n", topic_name, Full_Topic_Buffer);
    retval = MqttClient_obj.subscribe(Full_Topic_Buffer);
    if (!retval)
        dbprintf("subscribe(%s) failed\n", topic_name);
    return retval;
}

// parse the full topic of the received message
// return value: true=ok, false=Msg.fulltopic does not match the expected prefix/topic_name/suffix format
bool parse_topic(const char **topic_name_out, const char **suffix_out) {
    bool retval=false;
    if (Msg.fulltopic[0] != '/') {
        char *topic_name=nullptr;
        char *suffix=nullptr;
        memset(Full_Topic_Buffer, '\0', sizeof(Full_Topic_Buffer));
        strncpy(Full_Topic_Buffer, Msg.fulltopic, sizeof(Full_Topic_Buffer));
        char *slash_ptr = strchr(Full_Topic_Buffer, '/');
        if (slash_ptr) {
            topic_name=slash_ptr+1;
            slash_ptr = strchr(topic_name, '/');
            if (slash_ptr) {
                *slash_ptr='\0'; // terminate topic_name
                suffix=slash_ptr+1;
                retval=true;

                *topic_name_out=topic_name;
                *suffix_out=suffix;
            }
        }
    }
    return retval;
}

// reset MCU or hold program in infinite loop after fatal error
void EndProgram(const char *error_message, bool reset) {
	if (error_message)
		dbprintln(error_message);
	if (reset) {
        dbprintln("Reset");
		ESP.restart();
		// this point is never reached
	}
	else {
		const int HALTED_DELAY=60000; // ms
		while (1) {
			dbprintln("Program halted");
			if (BUILTIN_LED) {
				for (uint8_t idx=0; idx<3; idx++) {
					digitalWrite(BUILTIN_LED, HIGH);
					delay(2000);
					digitalWrite(BUILTIN_LED, LOW);
					delay(2000);
				}
			}
			else
				delay(HALTED_DELAY*1000);
		}
	}
}

// Note: Do not use the client in the callback to publish, subscribe or
// unsubscribe as it may cause deadlocks when other things arrive while
// sending and receiving acknowledgments. Instead, change a global variable,
// or push to a queue and handle it in the loop after calling `MqttClient_obj.loop()`.
// topics larger than FULLTOPIC_LEN or payloads larger than PAYLOAD_LEN will be truncated when copied to Msg
void onMsg(MQTTClient *client, char fulltopic[], char payload[], int length) {
    if (Msg.available) {
		// this message arrived before the processing of the previous one is complete
		dbprintf("Message collision: %s : discarded\n", fulltopic);
	}
	else {
		// copy payload to Msg.payload and make it a c-string
		memset(Msg.payload, '\0', sizeof(Msg.payload));
		memcpy(Msg.payload, payload, length<sizeof(Msg.payload)?length:sizeof(Msg.payload)-1);
        // copy fulltopic to Msg.fulltopic
		strncpy(Msg.fulltopic, fulltopic, sizeof(Msg.fulltopic));
		// a new message is available
		Msg.available=true;
	}
}

// if OTA_ENABLED then OTA.OTADelay() will be called instead, 
// and it will call ArduinoOTA.handle(), which is required for OTA.
// * make sure to call this function periodically in function loop(), 
// * if you don't need any delay in your loop() call it anyway with  millisec_lng = 0 
#ifndef OTA_ENABLED
// same as delay(), blocking but interruption friendly
void InterruptDelay(unsigned long millisec_lng) {
    if (millisec_lng) {
        unsigned long start_millis=millis();
        while (millis()-start_millis < millisec_lng)
            yield(); // required for ESP8266, NOP for other SOCs
    }
}
#endif
