// NoAckTx.ino - Transmit data to a receiver using automatic acknowledgement (we do not expect custom ACK datagrams from the receiver)
// The transmitter uses the automatic acknowledgement sent by the receiver to test if a transmitted packet has been received,
// and sends it again if no acknowledgement packet was received, according to parameters ART_ATTEMPTS and ART_DELAY.
// If transmission errors are acceptable then set ART_ATTEMPTS=0 to disable auto retransmission entirely.
// 2024-12-10

#include <SPI.h>
#include <RF24.h>
#include "Datagrams.h"

// instanciate an object for the nRF24L01 transceiver
RF24 Radio(CE_PIN, CSN_PIN, SPI_SPEED);

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
float Datagram = 0.0;

void setup() {
  Serial.begin(115200);
  while (!Serial) ;
  Serial.print('\n'); for (byte idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } ; Serial.print('\n');

  // initialize the transceiver on the SPI bus
  if (!Radio.begin()) {
    Serial.println("transceiver is not responding");
    while (1);
  }
  
  // Set the RF power output and Enable the LNA (Low Noise Amplifier) Gain
  Radio.setPALevel(PA_LEVEL);

  // Set the static payload length (must be identical in Tx and Rx)
  Radio.setPayloadSize(sizeof(Datagram));

  // Set the Transmission IDs
  Radio.openWritingPipe(TxId);    // device transmits on pipe 0
  Radio.openReadingPipe(1, RxId); // device receives on pipe 1

  Radio.setChannel(CHANNEL);
  Radio.stopListening();  // put device in TX mode

  // Auto retransmission:
	//  The RF24 chip is capable to retransmit datagrams ART_ATTEMPTS times
	//  if an ACK has not been received after ART_DELAY microseconds
  //  These settings do not apply to the receiver (calling setRetries() in RX mode has no effect)
	//
	// ART_ATTEMPTS:
	// 	To reduce the transmission error rate, set ART_ATTEMPTS (ART=auto retransmission) between 1 and 15
	// 	 take into account each retransmission will take ART_DELAY µs, reducing the time available for your application data processing
	// 	Alternatively, if transmission errors are acceptable then set ART_ATTEMPTS=0 to disable auto retransmission entirely
  //  Default Auto Retry Attempts	= 15
	const uint8_t ART_ATTEMPTS=5;
  //
	// ART_DELAY:
	// 	This delay must be larger than the normal MSG datagram transmission time + ACK datagram receiving time
  //  (1040 µs with my setup)
	// 	ART_DELAY: 0=250µs, 1=500µs, 2=750µs, 3=1000µs, 4=1250µ, 5=1500µs, ... 15=4000µs
  //  ignored if ART_ATTEMPTS=0
  //  Default Auto Retry Delay	= 1500 microseconds
	const uint8_t ART_DELAY=5;
	Radio.setRetries(ART_DELAY, ART_ATTEMPTS);  

  Radio.printPrettyDetails(); // (larger) function that prints human readable data
}

void loop() {
  unsigned long start_timer = micros();
  bool report = Radio.write(&Datagram, sizeof(Datagram));
  unsigned long end_timer = micros();

  if (report) {
    Serial.print("Time to transmit = ");
    Serial.print(end_timer - start_timer);
    Serial.print(" us. Sent: ");
    Serial.println(Datagram);
  } else
    Serial.println("Transmission failed or timed out");  // payload was not delivered

  Datagram += 0.01;
  delay(1000);
}
