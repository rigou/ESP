/* rgDisplayAsyncTest.ino 
08:48:16.896 -> AsyncPrintLine() "The quick"	23 micros
08:48:17.893 -> AsyncPrintLine() "brown fox"	23 micros
08:48:18.923 -> AsyncPrintLine() "jumps over"	24 micros
08:48:19.919 -> AsyncPrintLine() "the dog"	    23 micros
*/

#include <Arduino.h>

#define DEBUG_ON 1
#include <rgDebug.h>

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <rgDisplay.h>
rgDisplay Display_obj;

// Autorepeat hardware timer used to transmit datagrams periodically
// see https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html

hw_timer_t * Timer_obj = NULL;
volatile SemaphoreHandle_t Semaphore_obj;

// DGPERIOD is the delay between 2 data processings in loop(), in microseconds
// Acceptable CPU speed for DGPERIOD=10000 : 80-240 MHz on Tx and/or Rx
// Larger delays increase the time available for your application data processing between datagrams
const unsigned long DGPERIOD=10000; // microseconds, must be multiple of 100

void ARDUINO_ISR_ATTR onTimer(){
    xSemaphoreGiveFromISR(Semaphore_obj, NULL);
}

// Initialization:
void setup() {
    Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	dbprint('\n'); for (uint8_t idx = 0; idx<3; idx++) { dbprint((char)('A'+idx)); delay(500); }
	dbprint('\n'); 

    if (!Display_obj.Init(true)) {
		dbprintln("Display initialization failed");
        while (1);
    }

    // this semaphore tells when the timer has fired
    // we use it to make the onTimer() ISR return as fast as possible
    Semaphore_obj = xSemaphoreCreateBinary();
    // Set timer frequency to 10 kHz to get a 100 microsecond resolution (slower frequency won't work)
    Timer_obj = timerBegin(10000);
    // Attach onTimer function to our timer.
    timerAttachInterrupt(Timer_obj, &onTimer);
    // Set alarm to call onTimer function every DGPERIOD microseconds
    // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter)
    // onTimer() will be called when the counter of Timer_obj reaches this value, and the counter will be reset to 0
    timerAlarm(Timer_obj, DGPERIOD/100, true, 0);
}

// loopTask() runs on core 1 with priority 1
void loop() {
    static const char *LINES[4]={"The quick", "brown fox", "jumps over", "a lazy dog"};
    // display app data periodically and asynchronously
    if (xSemaphoreTake(Semaphore_obj, 0) == pdTRUE) {
        // data processing takes place here
        static unsigned int Counter=0;
        if (++Counter % 100 == 0) {
            uint16_t idx=(Counter/100) % 5; // 0 1 2 3 4
            if (idx<4) {
                unsigned long timer=micros();
                Display_obj.AsyncPrintLine(LINES[idx], 1); // 23 microseconds
                dbprintf("AsyncPrintLine() \"%s\"\t%lu micros\n", LINES[idx], micros()-timer);
            }
            else
                Display_obj.AsyncClear();
        }
    }
}
