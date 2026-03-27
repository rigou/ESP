/* ESP32 SPI Slave Sketch
    This Arduino sketch will turn the built-in Led on or off
    upon receiving the corresponding command on the SPI bus
    using the VSPI controller:
    VSPI bus normally attached to pins 5, 18, 19, and 23
    HSPI bus normally mapped to pins 12 - 15

	ESP32 Master	ESP32 Slave
	MOSI (GPIO23)	MOSI (GPIO23)
	MISO (GPIO19)	MISO (GPIO19)
	SCLK (GPIO18)	SCLK (GPIO18)
	CS   (GPIO5)	CS (GPIO15)

    SPI clock frequency information : see spi_clock_frequency_information.txt
*/

#include <ESP32SPISlave.h>

#define APP_NAME "SPI_Slave"
#define APP_VERSION "v1.2.1"

const int SERIAL_BAUDRATE = 115200;

const byte LED_GPIO=2;

// commands : same value in SPI_Master and SPI_Slave
const byte CMD_ERROR=0; // not transmitted to slave
const byte CMD_LED_OFF=1; // command to turn off LED of slave
const byte CMD_LED_ON=2;  // command to turn on LED of slave

const unsigned int SPI_BUFFER_SIZE=4; // same value in SPI_Master and SPI_Slave
byte TxBuffer[SPI_BUFFER_SIZE];
byte RxBuffer[SPI_BUFFER_SIZE];

ESP32SPISlave Spi_obj;

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

    pinMode(LED_GPIO, OUTPUT);

    Spi_obj.setDataMode(SPI_MODE0);
    Spi_obj.begin(VSPI);

    memset(TxBuffer, 0, SPI_BUFFER_SIZE);
    memset(RxBuffer, 0, SPI_BUFFER_SIZE);
}

void loop() {
    static byte Query_count=0;
    static byte Query_command=CMD_ERROR;
    static bool Led_state=LOW;
    static byte Result=0;

    // prepare our response, it will be sent while receiving next query from master
    // Query record format:    [0]=Query_count, [1]=command
    // Response record format: [0]=Query_count, [1]=command, [2]=Led_state, [SPI_BUFFER_SIZE-1]=result (0=error, 1=success)
    memset(TxBuffer, 0, SPI_BUFFER_SIZE);
    TxBuffer[0]=Query_count; // query number of previous transaction received
    TxBuffer[1]=Query_command; // previous command executed
    TxBuffer[2]=Led_state?1:0; // slave's state information after executiong previous command (add your own state info here)
    TxBuffer[SPI_BUFFER_SIZE-1]=Result; // result of previous command

    // block until the query comes from master
    Spi_obj.wait(RxBuffer, TxBuffer, SPI_BUFFER_SIZE);

    // if transaction has completed from master,
    // available() returns size of results of transaction,
    // and buffer is automatically updated
    while (Spi_obj.available()) {
        Query_count=RxBuffer[0];
        Query_command=RxBuffer[1];
        Spi_obj.pop();
    }

    bool valid_query=(Query_command==CMD_LED_ON || Query_command==CMD_LED_OFF);
    
    if (valid_query) {
        Led_state=(Query_command==CMD_LED_ON);
        digitalWrite(LED_GPIO, Led_state);
        Serial.printf("rx [%03d] cmd=%d -> led %d\n", Query_count, Query_command, Led_state);
        Result=1; // success
    }
    else {
        Serial.printf("rx: [%03d] cmd=%d error\n", Query_count, Query_command);
        Result=0; // error
    }
}
