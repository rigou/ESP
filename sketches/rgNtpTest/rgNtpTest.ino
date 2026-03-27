// rgNtpTest.ino
// copilot: write a program named "rgNtpTest" for the arduino esp32 that gets the time and date
// from the internet and prints it on the serial console. Do not use any led.
// 2025-09-21

#include <WiFi.h>
#include <time.h>
#include "/home/rigou/Projects/nogit/NetworkCredentials/NetworkCredentials.h"

#define APP_NAME "rgNtpTest"
#define APP_VERSION "v1.0.0"
#define SERIAL_BAUDRATE 115200

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;     // UTC
const int daylightOffset_sec = 0;  // No daylight saving time

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  
  // Print date and time in a readable format
  Serial.printf("%02d/%02d/%04d ", 
    timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  Serial.printf("%02d:%02d:%02d\n", 
    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial);
  
  // Print application info
  Serial.printf("\n%s %s\n", APP_NAME, APP_VERSION);
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(SSID_NET2, PASSWD_NET2);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  
  // Disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  printLocalTime();
  delay(1000);  // Wait for 1 second before printing again
}
