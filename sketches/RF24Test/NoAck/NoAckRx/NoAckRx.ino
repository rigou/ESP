// NoAckRx.ino - Receive data from a transmitter, using automatic acknowledgement (we do not send custom acknowledgements to the transmitter)
// 2024-12-10

#include <SPI.h>
#include <RF24.h>
#include "../NoAckTx/Datagrams.h"

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
  Radio.openWritingPipe(RxId);    // device transmits on pipe 0
  Radio.openReadingPipe(1, TxId); // device receives on pipe 1
  
  Radio.setChannel(CHANNEL);
  Radio.startListening();  // put device in RX mode

  Radio.printPrettyDetails(); // (larger) function that prints human readable data
}

void loop() {
  uint8_t pipe;
  if (Radio.available(&pipe)) {
    uint8_t bytes = Radio.getPayloadSize();
    Radio.read(&Datagram, bytes);
    Serial.print("Received ");
    Serial.print(bytes);
    Serial.print(" bytes on pipe ");
    Serial.print(pipe);
    Serial.print(": ");
    Serial.println(Datagram);
  }
}
