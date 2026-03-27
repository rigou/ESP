// Datagrams.h

#define CE_PIN 17
#define CSN_PIN 5
// Some transceivers does not initialize properly at default SPI speed of 10 Mhz :
// setChannel() sets a random channel instead of the specified one, or begin() fails 
// so we use 2 Mhz, see ARDUINO/Espressif/SPI_MasterSlave/doc/spi_clock_frequency_information.txt
#define SPI_SPEED 2000000

#define CHANNEL 3

// Set the PA Level low because this example is likely run with devices in close proximity to each other
// RF24_PA_MAX is default
#define PA_LEVEL RF24_PA_LOW

// Transmission ID used by Tx
const uint16_t TxId=0x24bc;

 // Transmission ID used by Rx
const uint16_t RxId=0x2e26;

// Message and Acknowledge datagram size, max 16 (32 bytes)
#define MSGVALUES 4
#define ACKVALUES 4

uint16_t Msg_Datagram[MSGVALUES]; // sent from Tx -> Rx
uint16_t Ack_Datagram[ACKVALUES]; // sent from Rx -> Tx

void PrintMsgDatagram(uint16_t datagram[]) {
	for (byte idx=0; idx<MSGVALUES; idx++)
		Serial.printf("x%04x ", datagram[idx]);
	Serial.println();
}

void PrintAckDatagram(uint16_t datagram[]) {
	for (byte idx=0; idx<ACKVALUES; idx++)
		Serial.printf("x%04x ", datagram[idx]);
	Serial.println();
}

// Extract the 1st "count" bytes (max 8) of "number" into array "bytes"
// if count > sizeof(number) then extra bytes are returned as 0x00
void GetBytes(uint8_t bytes[], uint64_t number, uint8_t count) {
	union {
		uint64_t number;
		uint8_t bytes[sizeof(number)];
	} result;
	result.number=number;
	for (int idx=0; idx<count; idx++)
		bytes[idx]=result.bytes[idx];
}
