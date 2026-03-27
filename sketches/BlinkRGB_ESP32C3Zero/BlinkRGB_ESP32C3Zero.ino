/* BlinkRGB_ESP32C3Zero.ino

  Demonstrates usage of onboard RGB LED of ESP32-C3-Zero

  Sources:
  https://www.waveshare.com/wiki/ESP32-C3-Zero#Introduction
  https://www.waveshare.com/wiki/ESP32-C3-Zero#Demo

  Arduino IDE: 
  Select board ESP32C3 Dev Module, Enable Tools/Use CDC on boot
  Upload: ESP32-C3-Zero does not employ a USB to UART chip. When flashing firmware, press the BOOT button once (GPIO9) before uploading your code

  RGB Led control:
  ESP32-C3-Zero uses GPIO10 to connect with WS2812 RGB LED.
  Control RGB led with Adafruit_NeoPixel library's neopixelWrite()
  void neopixelWrite(uint8_t pin, uint8_t red_val, uint8_t green_val, uint8_t blue_val)
  After using neopixelWrite() to drive RGB LED it will be impossible to drive the same pin with digitalWrite() with normal HIGH/LOW level
*/
#define RGB_BRIGHTNESS 10 // Change white brightness (max 255)

#ifdef RGB_BUILTIN
#undef RGB_BUILTIN
#endif
#define RGB_BUILTIN 10

void setup() {
  // No need to initialize the RGB LED
}

// the loop function runs over and over again forever
void loop() {
#ifdef RGB_BUILTIN
  // digitalWrite(RGB_BUILTIN, HIGH);   // Turn the RGB LED white
  neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,RGB_BRIGHTNESS,RGB_BRIGHTNESS); // Red
  delay(1000);
  // digitalWrite(RGB_BUILTIN, LOW);    // Turn the RGB LED off
  neopixelWrite(RGB_BUILTIN,0,0,0); // Red
  delay(1000);

  neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red
  delay(1000);
  neopixelWrite(RGB_BUILTIN,0,RGB_BRIGHTNESS,0); // Green
  delay(1000);
  neopixelWrite(RGB_BUILTIN,0,0,RGB_BRIGHTNESS); // Blue
  delay(1000);
  neopixelWrite(RGB_BUILTIN,0,0,0); // Off / black
  delay(1000);
#endif
}
