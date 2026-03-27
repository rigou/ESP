/* rgDisplayAscii.h - Print text on a SSD1306 OLED display connected to I2C
*/

#pragma once

#include <SPI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define DISPLAYLIB_NAME	"DisplayAscii" // spaces not permitted
#define DISPLAY_VERSION	"v2.0.5"

class rgDisplay : public SSD1306AsciiWire {
	public:
        // maximum size of text buffer is 172, corresponding to 8 lines of 21 char + 3 newlines + final \0
        static const unsigned short DISPLAY_LENGTH=172;
        
        rgDisplay(int address, int width, int height, byte reset_gpio=-1);
        int Init(void);
        void Clear(void);
        void Print(const char *text, byte font_size=1);
        void PrintAt(const char *text, int x_pos=0, int y_pos=0, byte font_size=1);

    private:
        int mAddress=0;
	byte mResetGpio=0;
 };
