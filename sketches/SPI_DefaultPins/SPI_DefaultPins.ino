/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-spi-communication-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#define APP_NAME "SPI_DefaultPins"
#define APP_VERSION "v1.0.0"

const int SERIAL_BAUDRATE = 115200;

//Find the default SPI pins for your board
//Make sure you have the right board selected in Tools > Boards
void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial) ; // wait for serial port to connect
  // give user some time to open the serial monitor
  int count_int = 0;
  Serial.println("");
  while (count_int < 5) {
    Serial.printf("%c", 'A' + count_int++);
    delay(1000);
  }
  Serial.printf("\n%s %s\n", APP_NAME, APP_VERSION);

  Serial.print("MOSI GPIO: ");
  Serial.println(MOSI);
  Serial.print("MISO GPIO: ");
  Serial.println(MISO);
  Serial.print("SCK GPIO: ");
  Serial.println(SCK);
  Serial.print("SS GPIO: ");
  Serial.println(SS);  
}

void loop() {
  // put your main code here, to run repeatedly:
}
