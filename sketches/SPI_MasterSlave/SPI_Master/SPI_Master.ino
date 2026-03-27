/* ESP32 SPI Master Sketch
    This Arduino sketch will transfer 1-byte commands on the SPI bus 
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

#include <SPI.h>

#define APP_NAME "SPI_Master"
#define APP_VERSION "v1.2.1"

const int SERIAL_BAUDRATE = 115200;
const int SPI_FREQ = 1000000; // 1 MHz ; the SPI slaves are designed to operate at up to 10 MHz

const byte SLAVE_SELECT_GPIO=SS;

// commands : same value in SPI_Master and SPI_Slave
const byte CMD_ERROR=0; // not transmitted to slave
const byte CMD_LED_OFF=1; // command to turn off LED of slave
const byte CMD_LED_ON=2;  // command to turn on LED of slave

const unsigned int SPI_BUFFER_SIZE=4; // same value in SPI_Master and SPI_Slave
byte SpiBuffer[SPI_BUFFER_SIZE];

SPIClass * Spi_obj = NULL;

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

    Spi_obj = new SPIClass(VSPI);
    //Spi_obj->begin(); // same as Spi_obj->begin(SCK, MISO, MOSI, SS)
    Spi_obj->begin(SCK, MISO, MOSI, SLAVE_SELECT_GPIO);

    // set up slave select pins as output - see select_slave() below
    pinMode(SLAVE_SELECT_GPIO, OUTPUT);
}

// the loop function runs over and over again until power down or reset
void loop() {
    send_command();
}

void send_command() {
    static byte Command=CMD_LED_OFF;
    unsigned int command_delay=0;

    switch (Command) {
        case CMD_LED_OFF:
            Command=CMD_LED_ON;
            command_delay=1000;
            break;

        case CMD_LED_ON:
            Command=CMD_LED_OFF;
            command_delay=1000;
            break;
        
        default:
            Command=CMD_ERROR;
            command_delay=1000;
            Serial.printf("invalid command %d\n", Command);
            break;
    }
    if (Command!=CMD_ERROR)
        transmit_spi(Command); // we ignore the returned value in this example
    delay(command_delay);
}

// transmit given command and receive the slave's response for the previous call to transmit_spi()
// we use only 2 bytes of the transmit buffer in this example
// the result of this command will be returned by next call to transmit_spi()
// Query record format:    [0]=Query_count, [1]=command
// Response record format: [0]=Query_count, [1]=command, [2]=Led_state, [SPI_BUFFER_SIZE-1]=result (0=error, 1=success)
// return value: the slave's result byte for the previous command, more state information is available in SpiBuffer
byte transmit_spi(const byte query_command) {
    static byte Query_count=0;

    // format our query
    memset(SpiBuffer, 0, SPI_BUFFER_SIZE);
    SpiBuffer[0]=Query_count;
    SpiBuffer[1]=query_command; 

    Spi_obj->beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE0));
    select_slave(true);
    // the SPI controller sends the query and receives the response simultaneaously, bit by bit
    // at this point SpiBuffer contains our query
    Spi_obj->transfer(SpiBuffer, SPI_BUFFER_SIZE);
    // at this point SpiBuffer contains the response of the slave
    select_slave(false);
    Spi_obj->endTransaction();

    byte response_count=SpiBuffer[0];
    byte response_command=SpiBuffer[1];
    byte response_state=SpiBuffer[2];
    byte response_result=SpiBuffer[SPI_BUFFER_SIZE-1]; // 1=success, 0=error

    bool valid_response=((Query_count==response_count+1) || (Query_count==0 && response_count==255))
        && (response_command==CMD_LED_ON || response_command==CMD_LED_OFF)
        && (response_state==0 || response_state==1)
        && (response_result==0 || response_result==1);

    if (valid_response)
        Serial.printf("[%03d] tx %d -> rx: [%03d] cmd=%d, led=%d, result=%d\n", 
            Query_count, query_command, response_count, response_command, response_state, response_result);
    else {
        Serial.printf("[%03d] tx %d -> rx: [%03d] error\n", Query_count, query_command, response_count);
        response_result=0;
    }

    Query_count++;
    return response_result;
}

void select_slave(bool selected) {
    digitalWrite(SLAVE_SELECT_GPIO, !selected); // set it LOW to select the slave
}
