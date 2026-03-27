/*  SerialioTest - Derived from example code https://www.arduino.cc/en/Tutorial/BuiltInExamples/PhysicalPixel
*/

const int ledPin = 2; // the pin that the LED is attached to
int incomingByte;      // a variable to read incoming serial data into

void setup() {
	Serial.begin(9600);
	while (!Serial) ; // wait for serial port to connect   
	Serial.print('\n'); for (uint8_t idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } // debug
	Serial.println("");
	Serial.println("");
	Serial.println("SERIAL I/O TEST");
	Serial.printf("Led is on GPIO %d\n", ledPin);
	Serial.println("Enter H to turn Led On, Enter L to turn Led Off");
	
	pinMode(ledPin, OUTPUT);
}

void loop() {
  // see if there's incoming serial data:
  if (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    incomingByte = Serial.read();
    // if it's a capital H (ASCII 72), turn on the LED:
    if (incomingByte == 'H') {
      digitalWrite(ledPin, HIGH);
	Serial.println("Led is On");
    }
    // if it's an L (ASCII 76) turn off the LED:
    if (incomingByte == 'L') {
      digitalWrite(ledPin, LOW);
	Serial.println("Led is Off");
    }
  }
}
