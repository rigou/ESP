// Datagrams.h

#define CE_PIN 17
#define CSN_PIN 5
// Some transceivers does not initialize properly at default SPI speed of 10 Mhz :
// setChannel() sets a random channel instead of the specified one, or begin() fails 
// so we use 2 Mhz, see ARDUINO/Espressif/SPI_MasterSlave/doc/spi_clock_frequency_information.txt
#define SPI_SPEED 2000000

#define CHANNEL 7

// Set the PA Level low because this example is likely run with devices in close proximity to each other
// RF24_PA_MAX is default
#define PA_LEVEL RF24_PA_LOW

// Transmission ID used by Tx
const uint8_t TxId[]={'3','3','3','3','3'}; // 0x3333333333

 // Transmission ID used by Rx
const uint8_t RxId[]={'D','D','D','D','D'}; // 0x4444444444
