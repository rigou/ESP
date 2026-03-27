/**
 * Channel scanner and Continuous Carrier Wave detection
 *
 * Example to detect interference on the various channels available.
 * This is a good diagnostic tool to check whether you're picking a
 * good channel for your application.
 *
 * 1. Scan all channels from channel 0 to channel NUM_CHANNELS-1 :
 * Run this sketch on a single device and watch the results :
 * - free channels should display measurement '-'
 * - noisy channels should display measurement > 0
 *
 * 2. Test if 2 devices communicate :
 * Run this sketch on two devices. On one device, start carrier output by sending a 'g'
 * character over Serial. The other device scanning should detect the output of the sending
 * device on channel CARRIER_CHAN showing a constant measurement of 'F'. Values less than 'F'
 * indicate that the channel is too noisy.
 * Stop carrier output by sending a 'e' character over Serial.
 * 
* Ported to ESP32 and improved by Richard Goutorbe 2023-12-20, 2024-06-02
 * 
 * Inspired by cpixip.
 * See http://arduino.cc/forum/index.php/topic,54795.0.html
 */

/* Output sample :
000000000000000011111111111111112222222222222222333333333333333344444444444444445555
0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123
----------1----222343334232111-------11-------------------------------111-----------
-----11--22------333332324531-1-----------------------------------------------------
---------32-1--111433144354432------------1----------11----1------------------------
-----5---11----111332333432121--------1----11-----------1-----------1----1----------
---------11-1--11433332233432-1-------------------------1-----------1-------------1-
----------------2443321223431-------------------1---1-----1----1--------------------
--1--1-----------1223233232111--------------------------------------------------2---
-1--11----------23333342343223--1-1--1---------------1--1---1-----1--1----------1---
----11-----------3233313212-11------------------------------------------------------
-----11--------11--22113333423--------------------------------------------------1---
*/
#include "RF24.h"

//
// Hardware configuration
//

#define CE_PIN 4
#define CSN_PIN 5
const unsigned int SPI_SPEED=2000000; // Hz, see ARDUINO/Espressif/SPI_MasterSlave/doc/spi_clock_frequency_information.txt
// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN, SPI_SPEED);

//
// Channel info
//
const uint8_t NUM_CHANNELS = 126;
uint8_t values[NUM_CHANNELS];
const uint8_t CARRIER_CHAN = 64; // Max 83 in France

const int NUM_REPS = 100;
int LineNumber=0;
bool TxDevice=false;

//
// Setup
//

void setup(void) {
  //
  // Print preamble
  //

  Serial.begin(115200);
  Serial.println("\n\rRF24/examples/scanner/");

  //
  // Setup and configure rf radio
  //

  radio.begin();
  	if (!radio.begin()) {
		  Serial.println("Radio hardware is not responding");
      while (true);
	}

  radio.setAutoAck(false);

  // Get into standby mode
  radio.startListening();
  radio.stopListening();
  radio.printDetails();
  Serial.println("");
  Serial.print("Type 'g' to start transmitting Carrier on channel ");
  Serial.println(CARRIER_CHAN);
  Serial.println("Type 'e' to stop transmitting Carrier");
  Serial.println("----------------------------------------");
  radio.printPrettyDetails(); // debug : print human readable data
	Serial.println("----------------------------------------");
	Serial.println("Scanning...");
}

//
// Loop
//

void loop(void) {
  // Send g over Serial to begin carrier output
  // Configure power level below
  if (Serial.available()) {
    char c = Serial.read();
    Serial.println();
    if (c == 'g') {
      TxDevice=true;
      radio.stopListening();
      delay(2);
      Serial.print("Transmitting Carrier on channel ");
      Serial.println(CARRIER_CHAN);
      radio.startConstCarrier(RF24_PA_LOW, CARRIER_CHAN);
    }
    else if (c == 'e' && TxDevice) {
      radio.stopConstCarrier();
      Serial.println("Stopping Carrier");
    }
  }

  if (TxDevice == false) {
  
    // Clear measurement values
    memset(values, 0, sizeof(values));

    // Scan all channels NUM_REPS times
    int rep_counter = NUM_REPS;
    while (rep_counter--) {
      int i = NUM_CHANNELS;
      while (i--) {
        // Select this channel
        radio.setChannel(i);

        // Listen for a little
        radio.startListening();
        delayMicroseconds(128);
        radio.stopListening();

        // Did we get a carrier?
        if (radio.testCarrier()) {
          ++values[i];
        }
      }
    }

    if (LineNumber==0 || LineNumber==10) {
        print_header();
        LineNumber=0;
    }
    LineNumber++;

    // Print out channel measurements, clamped to a single hex digit
    int i = 0;
    while (i < NUM_CHANNELS) {
      if (values[i]) {
        uint8_t value=values[i];
        if (value>0xf)
          value=0xf;
        Serial.print(value, HEX);
      }
      else
        Serial.print('-');

      ++i;
    }
    Serial.println();
  }
}

void print_header() {
// Print out header, high then low digit
  int i = 0;
  while (i < NUM_CHANNELS) {
    Serial.print(i >> 4, HEX);
    ++i;
  }
  Serial.println();
  i = 0;
  while (i < NUM_CHANNELS) {
    Serial.print(i & 0xf, HEX);
    ++i;
  }
  Serial.println();
}

