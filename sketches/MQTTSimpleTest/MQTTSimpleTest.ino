// MQTTSimpleTest.ino - arduino-mqtt library usage example
// this program uses the arduino-mqtt library by Joël Gähwiler, which is installed in Projects/Arduino/libraries/MQTT
// (install it with the Arduino IDE Library manager)
// https://github.com/256dpi/arduino-mqtt
// 2026-01-06

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
#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

#include "MQTTSimpleTest.h"

WiFiClient WifiClient_obj;

/* The maximum size for packets being published and received is set by default to 128 bytes. 
    The receiving of messages is constrained by the read buffer, which may be increased if necessary.
    To change the read buffer size, you need to use MQTTClient MqttClient_obj(256) or MQTTClient MqttClient_obj(256, 128)
    (128 bytes is enough for the write buffer because the message payload is sent directly during publishing)
*/
MQTTClient MqttClient_obj;

void setup() {
    Serial.begin(921600);
    while (!Serial);
#if DEBUG_ON
	dbprint('\n'); for (uint8_t idx=0; idx<12; idx++) { dbprint((char)('A'+idx)); delay(500); }
#endif
	dbprintf("\n\n%s %s\n", APP_NAME, APP_VERSION);
	
    MqttClient_obj.onMessageAdvanced(messageReceivedAdvanced);
    MqttClient_obj.begin(MQTT_HOST, WifiClient_obj); // public.cloud.shiftr.io for testing
}

void loop() {
    bool connected = WiFi.status() == WL_CONNECTED && MqttClient_obj.connected();

    if (!connected)
        connected=connect();

    if (connected) {
        MqttClient_obj.loop();

        // publish a message every second
        static unsigned long Last_status_update=0;
        if (Last_status_update==0 || millis()-Last_status_update>=1000) {
            MqttClient_obj.publish("hello", "world");
            Last_status_update=millis();
        }
    }
    else
        inter_friendly_delay(5000);
}

bool connect() {
    bool retval=false;
    if (WiFi.status() != WL_CONNECTED) {
        dbprintf("connecting to WiFi %s\n", WIFI_SSID);
        WiFi.begin(WIFI_SSID, WIFI_PASSWD);
        inter_friendly_delay(2500); // it takes up to 2s to have a valid WiFi.status()
    }
    if (WiFi.status() == WL_CONNECTED) {
        if (!MqttClient_obj.connected()) {
            dbprintf("connecting to MQTT broker %s as %s\n", MQTT_HOST, MQTT_CLID);
            if (MqttClient_obj.connect(MQTT_CLID, MQTT_USER, MQTT_PASSWD)) {
                // in this simple example we subscribe to the same topic that we publish,
                // but in reality we don't do that: we would subscribe to different topic(s)
                MqttClient_obj.subscribe("hello");
                retval=true;
            }
            else
                dbprintln("MQTT broker connection failed");
        }
    }
    else
        dbprintln("WiFi connection failed");

    if (retval)
        dbprintln("connected");
    return retval;
}

// Note: Do not use the client in the callback to publish, subscribe or
// unsubscribe as it may cause deadlocks when other things arrive while
// sending and receiving acknowledgments. Instead, change a global variable,
// or push to a queue and handle it in the loop after calling `MqttClient_obj.loop()`.
void messageReceivedAdvanced(MQTTClient *client, char fulltopic[], char payload[], int length) {
    char payload_string[256];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    dbprintf("incoming: %s - %s\n", fulltopic, payload);
}

// same as delay(), blocking but interruption friendly
void inter_friendly_delay(unsigned long millisec_lng) {
	unsigned long start_millis=millis();
	while (millis()-start_millis < millisec_lng)
		yield(); // required for ESP8266, NOP for other SOCs
}
