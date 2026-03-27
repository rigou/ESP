// rgDisplay.h - Print text on a SSD1306 OLED display connected to I2C
#pragma once
#include <Adafruit_SSD1306.h>

#define DISPLAY_NAME	"rgDisplay" // spaces not permitted
#define DISPLAY_VERSION	"v4.0.1"

#define DEF_FONT_SIZE 2 // font_size 2 : 4 x 10     char height=16 char width=12 -> 4 lines 10 characters/line

class rgDisplay : public Adafruit_SSD1306 {
    public:
        rgDisplay(void);
        void set_FontSize(uint8_t font_size);
        uint8_t get_FontSize(void);
        bool Init(bool async_mode=false);
        void Clear(bool refresh=true);
        void Print(const char *text, bool refresh=true, uint8_t font_size=0);
        void PrintLine(const char *text, uint8_t line_number, bool refresh=true, uint8_t font_size=0);
        void PrintAt(const char *text, int x_pos=0, int y_pos=0, bool refresh=true, uint8_t font_size=0);
        void Refresh(void);

        bool AsyncClear(void);
        bool AsyncPrintLine(const char *text, uint8_t line_number, bool refresh=true, uint8_t font_size=0);

    private:
        // font_size 1 : 8 x 21
        // font_size 2 : 4 x 10     char height=16 char width=12 -> 4 lines 10 characters/line
        // font_size 3 : 2.9 x 7
        // font_size 4 : 2 x 5
        // font_size 5 : 1.6 x 4
        // font_size 6 : 1.3 x 3
        // font_size 7 : 1.1 x 3
        // font_size 8 : 1 x 2
        static const uint8_t MAXCOLS=21;
        const uint8_t FONT_COLS[8]={MAXCOLS, 10, 7, 5, 4, 3, 3, 2};
        const uint8_t FONT_LINES[8]={8, 4, 3, 2, 1, 1, 1, 1};
        
        uint8_t FontSize=DEF_FONT_SIZE; // 1-8
        static const size_t READQ_STACKSIZE=2048; // bytes, 1524 used
        
        // FreeRTOS queue used to communicate from loopTask to task_read_msg for performing PrintLine() asynchronously
        QueueHandle_t Qh;
        static const uint8_t QTEXTLEN=MAXCOLS;
        enum QCommand {CLEAR, PRINTLINE};
        typedef struct ReadQueueData {
            uint8_t command;
            uint8_t line_number;
            uint8_t refresh;
            uint8_t font_size;
            char text[QTEXTLEN+1];
        } ReadQData;

        static void async_read_command(void *argp);
};
