// use Greiman's SSD1306Ascii  Library from https://github.com/greiman/SSD1306Ascii
#include "rgDisplayAscii.h"

// GPIOS allocation on an ESP32 : SDA=21, SCL=22

// address of the OLED display on the I2C bus ; use I2C_Scanner.ino to find the correct address
// width OLED display width, in pixels
// height OLED display height, in pixels
// reset_gpio used to reset the OLED display if your OLED module has one, else -1
// Return value: 0=success, else error
rgDisplay::rgDisplay(int address, int width, int height, byte reset_gpio) {
	mAddress=address;
	mResetGpio=reset_gpio;
}

// Return value: 0=success, !0=error
int rgDisplay::Init(void) {
	int retval=0;
	Wire.begin();
	// Wire.setClock(400000L); // fast mode (default)
	Wire.setClock(1000000L); // fast mode plus
	// Wire.setClock(3400000L); // high speed mode
	// test if display device connected
	Wire.beginTransmission(mAddress);
    if (Wire.endTransmission()==0) {
		// connected
		if (mResetGpio!=-1)
			begin(&Adafruit128x64, mAddress, mResetGpio);
		else
			begin(&Adafruit128x64, mAddress);
		setFont(Adafruit5x7);
		setScrollMode(SCROLL_MODE_AUTO);
		clear();
	}
	else
		retval=1; // device not connected
	return retval;
}

void rgDisplay::Clear(void) {
	clear();
	setCursor(0,0);
}

// font_size 1 : 8 x 21
// font_size 2 : 4 x 10
// the time required to print a string of 8 characters depends on the CPU frequency: 
// 80 MHz = 10.8 ms ; 160 MHz = 8.4 ms ; 240 MHz = 7.5 ms (40 MHz = screen not working)
void rgDisplay::Print(const char *text, byte font_size) {
	static byte Current_size=0;
	if (font_size != Current_size) {
		if (font_size==1)
			set1X();
		else
			set2X();
		Current_size=font_size;
	}
	print(text);
}

// x_pos, y_pos in pixels, origin 0,0 is upper left corner
void rgDisplay::PrintAt(const char *text, int x_pos, int y_pos, byte font_size) {
	setCursor(x_pos, y_pos);
	Print(text, font_size);
}
