/* rgDisplay.cpp - Print text on a SSD1306 OLED display connected to I2C
	This class can display text asynchronously on 2-Cores ESP32 microcontrollers
  	It is designed to run in Core 1, except method async_read_command()
	which runs on Core 0 if asynchronous mode is enabled
*/

#include <Wire.h>
#include "rgDisplay.h"

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 0
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an ESP32 : 21(SDA), 22(SCL)
#define SCREEN_WIDTH  128 	// OLED display width, in pixels
#define SCREEN_HEIGHT  64 	// OLED display height, in pixels
#define OLED_RESET     -1 	// Reset pin not used
#define CLK_DURING 400000 	// Speed (in Hz) for Wire transmissions in SSD1306 library calls.
#define CLK_AFTER  100000 	// Speed (in Hz) for Wire transmissions following SSD1306 library calls.
#define SCREEN_ADDRESS 0x3C	// use I2C_Scanner.ino to find the correct address

rgDisplay::rgDisplay(void) : Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

// address of the OLED display on the I2C bus ; use I2C_Scanner.ino to find the correct address
// async_mode: true = perform commands asynchronously using async_read_command() in Core 0, 
// false = execute commands synchronously in core 1
// Return value: 0=success, !0=error
bool rgDisplay::Init(bool async_mode) {
	trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    bool retval=false;
	if (async_mode) {
		Qh = xQueueCreate(1, sizeof(ReadQData)); // depth=1
    	assert(Qh);

		// task task_read_msg() runs on core 0 with priority 1
		TaskHandle_t th;
	    BaseType_t rto_status;
    	rto_status = xTaskCreatePinnedToCore(
			async_read_command,
			"readline",
			READQ_STACKSIZE,
			this,
			1,        // same priority as loopTask
			&th,      // Task handle
			0         // Run on core 0
		);
		assert(rto_status == pdPASS);
		assert(th);
	}
 	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        setTextSize(FontSize); // char height=16 char width=12 -> 4 lines 10 characters/line
        setTextColor(SSD1306_WHITE);
        cp437(true); // IBM-PC OEM font
		Clear();
		retval=true;
    }
    return retval;
}

void rgDisplay::Clear(bool refresh) {
	trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
	clearDisplay();
	setCursor(0,0);
	if (refresh)
		display();
}

void rgDisplay::Refresh(void) {
	trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
	display();
}

// print text at the current location of the cursor
// refresh : optional, true=auto-refresh display (default), false=write text in screen buffer but do not refresh display
// font_size : optional, specify it to print with a size differing from FontSize
void rgDisplay::Print(const char *text, bool refresh, uint8_t font_size) {
	trprintf("*** %s %s(\"%s\", %d, %d) begin\n", __FILE_NAME__, __FUNCTION__, text, refresh, font_size);
	if (font_size==0)
		font_size=FontSize;
	if (font_size && font_size!=FontSize)
		setTextSize(font_size);
	setTextColor(WHITE, BLACK);
	print(text);
	if (font_size && font_size!=FontSize)
		setTextSize(FontSize);
	if (refresh)
		display();
}

// print text at the given location
// refresh : optional, true=auto-refresh display (default), false=write text in screen buffer but do not refresh display
// font_size : optional, specify it to print with a size differing from FontSize
void rgDisplay::PrintAt(const char *text, int x_pos, int y_pos, bool refresh, uint8_t font_size) {
	trprintf("*** %s %s(\"%s\", %d, %d, %d, %d) begin\n", __FILE_NAME__, __FUNCTION__, text, x_pos, y_pos, refresh, font_size);
	if (font_size==0)
		font_size=FontSize;
	setCursor(x_pos, y_pos); // origin 0,0 upper left corner
	Print(text, refresh, font_size);
}

// print text at the beginning of given line and clear to end of line
// line_number is 1-based, in the range (1, FONT_LINES[font_size-1])
// all lines must have the same height for correct vertical positionning
// refresh : optional, true=auto-refresh display (default), false=write text in screen buffer but do not refresh display
// font_size : optional, specify it to print with a size differing from FontSize
// Execution time: I2C clock=400 kHz : 29 ms with CPU freq=80 MHz, 26 ms with CPU freq=240 MHz
// setting a faster I2C clock with CLK_DURING has no effect on the execution time
void rgDisplay::PrintLine(const char *text, uint8_t line_number, bool refresh, uint8_t font_size) {
	trprintf("*** %s %s(\"%s\", %u, %d, %u) begin\n", __FILE_NAME__, __FUNCTION__, text, line_number, refresh, font_size);
	if (font_size==0)
		font_size=FontSize;

	uint8_t MAXLEN=FONT_COLS[font_size-1];
	char line_buffer[MAXLEN+1];
	int txtlen=strlen(text);
	if (txtlen>MAXLEN)
		txtlen=MAXLEN;
	memset(line_buffer, ' ', MAXLEN+1);
	memcpy(line_buffer, text, txtlen);
	line_buffer[FONT_COLS[font_size-1]]='\0';

	uint8_t y_pos=(line_number-1)*(SCREEN_HEIGHT/FONT_LINES[font_size-1]);
	PrintAt(line_buffer, 0, y_pos, refresh, font_size);
}

// set default FontSize for the whole display
// all Print*() method have an optional font_size argument to override it if needed
void rgDisplay::set_FontSize(uint8_t font_size) {
	trprintf("*** %s %s(%d) begin\n", __FILE_NAME__, __FUNCTION__, font_size);
	if (font_size>=1 && font_size<=8) {
		FontSize=font_size;
		setTextSize(font_size);
	}
}

uint8_t rgDisplay::get_FontSize(void) {
	trprintf("*** %s %s() returns %d\n", __FILE_NAME__, __FUNCTION__, FontSize);
	return FontSize;
}

bool rgDisplay::AsyncClear(void) {
	bool retval=false;
	ReadQData qdata;
	memset(&qdata, 0, sizeof(ReadQData));
	qdata.command=QCommand::CLEAR;
	
	retval=(xQueueSendToBack(Qh,&qdata,sizeof(ReadQData)) == pdPASS);
	return retval;
}

bool rgDisplay::AsyncPrintLine(const char *text, uint8_t line_number, bool refresh, uint8_t font_size) {
	bool retval=false;
	ReadQData qdata;
	qdata.command=QCommand::PRINTLINE;
	qdata.line_number=line_number; // 1 2 3 4
	qdata.refresh=!!refresh;
	qdata.font_size=font_size;
	memset(qdata.text, 0, QTEXTLEN+1);
	strncpy(qdata.text, text, QTEXTLEN);

	retval=(xQueueSendToBack(Qh,&qdata,sizeof(ReadQData)) == pdPASS);
	return retval;
}

// Queue receiving task, blocks reading the queue
// Stack usage: 1240 bytes
void rgDisplay::async_read_command(void *argp) {
	rgDisplay *display_obj=(rgDisplay *)argp; // arg 3 of xTaskCreatePinnedToCore() in Init()
    BaseType_t rto_status;
    ReadQData qdata;

    while (1) {
        rto_status = xQueueReceive(
            display_obj->Qh,
            &qdata,
            portMAX_DELAY
        );
        assert(rto_status == pdPASS);
        
		// display stack usage
		//~ static char stack_str[12];
		//~ unsigned stack_used=READQ_STACKSIZE - uxTaskGetStackHighWaterMark(nullptr);
		//~ snprintf(stack_str, 11, "%u/%u", stack_used, READQ_STACKSIZE);
		//~ display_obj->PrintLine(stack_str, 8, true, 1);
		//~ Serial.println(stack_str);

		switch ((QCommand)qdata.command) {
			case QCommand::CLEAR:
				display_obj->Clear();
				break;

			case QCommand::PRINTLINE:
				display_obj->PrintLine(qdata.text, qdata.line_number, qdata.refresh, qdata.font_size);
				break;
		}
    }
}

