// AckRx.ino - Receive data from a transmitter, bidirectional (we send custom ACK datagrams to the transmitter)
// 2024-12-01

#include <SPI.h>
#include <RF24.h>
#include "../AckTx/Datagrams.h"

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

	// Set CRC size
	Radio_obj.setCRCLength(RF24_CRC_16); // 16 bits is the default but let's be explicit

	// The transmission data rate affects the range and the transmission error rate
	Radio_obj.setDataRate(RF24_250KBPS);

	const uint8_t ADDRESS_WIDTH=3; // we want 2-byte addresses only but setAddressWidth() requires min value=3
	Radio_obj.setAddressWidth(ADDRESS_WIDTH);

	// Set the Transmission IDs
	uint8_t dev_id[ADDRESS_WIDTH]; 
	GetBytes(dev_id, RxId, ADDRESS_WIDTH);
	Radio_obj.openWritingPipe(dev_id);    // device transmits on pipe 0
	GetBytes(dev_id, TxId, ADDRESS_WIDTH);
	Radio_obj.openReadingPipe(1, dev_id); // device receives on pipe 1
	Serial.printf("Rx WritingPipe address=0x%06x Rx ReadingPipe address=0x%06x\n", RxId, TxId);

	Radio_obj.setChannel(CHANNEL);
	Radio_obj.startListening();  // put device in RX mode

	// initialize the first acknowledgement datagram for pipe 1
	// The next time a message is received on pipe 1, the data in Ack_Datagram will be sent back in the ACK payload
	memset(Ack_Datagram, 0, sizeof(Ack_Datagram));
	Radio_obj.writeAckPayload(1, Ack_Datagram, sizeof(Ack_Datagram));

	Radio_obj.printPrettyDetails(); // (larger) function that prints human readable data
}

void loop() {
  if (Radio_obj.available()) {
		Radio_obj.read(Msg_Datagram, sizeof(Msg_Datagram));  // get incoming payload and send outgoing ack datagram
		Serial.printf("%d bytes MSG: ", Radio_obj.getDynamicPayloadSize());
		PrintMsgDatagram(Msg_Datagram);

		// Prepare next outgoing ack datagram and store it in pipe 1,
		// it will be transmitted by next call to read()
		// and the transmitter will receive it in its pipe 0
		memset(Ack_Datagram, 0, sizeof(Ack_Datagram));
		Ack_Datagram[0]=Msg_Datagram[0];
		Ack_Datagram[1]=1;
		Ack_Datagram[2]=2;
		Ack_Datagram[3]=3;
		Radio_obj.writeAckPayload(1, Ack_Datagram, sizeof(Ack_Datagram));
		Serial.printf("%d bytes ACK: ", sizeof(Ack_Datagram));
		PrintAckDatagram(Ack_Datagram);
  }
}
