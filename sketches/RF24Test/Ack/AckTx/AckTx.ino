// AckTx.ino - Transmit data to a receiver, bidirectional (we expect custom ACK datagrams from the receiver)
// 2024-12-10

#include <SPI.h>
#include <RF24.h>
#include "Datagrams.h"

// instanciate an object for the nRF24L01 transceiver
RF24 Radio_obj(CE_PIN, CSN_PIN, SPI_SPEED);

void setup() {
	Serial.begin(115200);
	while (!Serial) ;
	Serial.print('\n'); for (byte idx = 0; idx<8; idx++) { Serial.print((char)('A'+idx)); delay(500); } ; Serial.print('\n');

	// initialize the transceiver on the SPI bus
	if (!Radio_obj.begin()) {
		Serial.println("transceiver is not responding");
		while (1);
	}

	// Set the RF power output and Enable the LNA (Low Noise Amplifier) Gain
	Radio_obj.setPALevel(PA_LEVEL, true);

	// Enable custom payloads in the acknowledgement datagrams
	// this will automatically enable dynamic payloads on pipe 0 (required for TX mode when expecting ACK payloads) & pipe 1. 
	Radio_obj.enableAckPayload();

	const uint8_t ADDRESS_WIDTH=3; // we want 2-byte addresses only but setAddressWidth() requires min value=3
	Radio_obj.setAddressWidth(ADDRESS_WIDTH);

	// Set the Transmission IDs
	uint8_t dev_id[ADDRESS_WIDTH]; 
	GetBytes(dev_id, TxId, ADDRESS_WIDTH);
	Radio_obj.openWritingPipe(dev_id);    // device transmits on pipe 0
	GetBytes(dev_id, RxId, ADDRESS_WIDTH);
	Radio_obj.openReadingPipe(1, dev_id); // device receives on pipe 1
	Serial.printf("Tx WritingPipe address=0x%06x Rx ReadingPipe address=0x%06x\n", TxId, RxId);

	// Set CRC size
	Radio_obj.setCRCLength(RF24_CRC_16); // 16 bits is the default but let's be explicit

	// The transmission data rate affects the range and the transmission error rate
	Radio_obj.setDataRate(RF24_250KBPS);

	Radio_obj.setChannel(CHANNEL);
  	Radio_obj.stopListening();  // put device in TX mode

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
	Radio_obj.setRetries(ART_DELAY, ART_ATTEMPTS);  

	Radio_obj.printPrettyDetails(); // (larger) function that prints human readable data
}

void loop() {
	static uint16_t Counter_int=0; // 0 - 65535
	unsigned long start_timer=micros(); // start the timer
	byte status=0; // 0=success

	memset(Msg_Datagram, 0, sizeof(Msg_Datagram));
	Msg_Datagram[0]=Counter_int++;
	Msg_Datagram[1]=1;
	Msg_Datagram[2]=2;
	Msg_Datagram[3]=3;
	if (Radio_obj.write(Msg_Datagram, sizeof(Msg_Datagram))) {
		if (Radio_obj.available()) {  // is there an ACK payload ? grab the pipe number that received it
			Radio_obj.read(Ack_Datagram, sizeof(Ack_Datagram));  // get incoming ACK payload
			status=0; // good ACK datagram received
		} 
		else
			status=1;  // empty ACK datagram received
	}
	else
		status=2;  // payload was not delivered

	// successfull transmission time=1922 us, 1 retry=3851 us for SPI=2MHz, MCU=80 MHz, RF24_250KBPS
	Serial.printf("%lu us %d bytes MSG: ", micros() - start_timer, sizeof(Msg_Datagram));
	PrintMsgDatagram(Msg_Datagram);
	switch(status) {
	case 0:
		Serial.printf("%d bytes ACK: ", Radio_obj.getDynamicPayloadSize());
		PrintAckDatagram(Ack_Datagram);
		break;
	case 1:
		Serial.println(" Received an empty ACK datagram");
		break;
	case 2:
		Serial.println(" Transmission failed or timed out");
		break;
	}
	delay(1000);
}
