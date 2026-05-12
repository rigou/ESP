// TimerThreadv2.ino - This example shows how to use a hardware timer on a separate task in ESP32,
// with all time critical code isolated in separate module TimerThreadModule.cpp
// 2026-04-19

// a more efficient implementation is described here:
// https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/03-Direct-to-task-notifications/02-As-binary-semaphore

#include <Arduino.h>
#include "TimerThreadModule.h"

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

#ifndef BUILTIN_LED
#define BUILTIN_LED 2
#endif

// timertsk() sets these global variables, loop() reads them
extern uint32_t IsrCounter;
//extern uint32_t LastIsrAt; // unused
// this spinlock controls the access to these global variables
extern portMUX_TYPE timerMux_obj;

void setup() {
    Serial.begin(921600);
    while (!Serial);
    dbprint('\n'); for (uint8_t idx=0; idx<13; idx++) { dbprint((char)('A'+idx)); delay(500); }
    dbprint('\n');

    pinMode(BUILTIN_LED, OUTPUT);
    ttm_Setup();

    Serial.print("main loop running on core ");
    Serial.print(xPortGetCoreID());
    dbprintln(" ; main setup complete");
}

// loop() processes actions that are not time-critical
void loop() {
    static bool Led_state=false;
    static uint32_t Last_isr_count=0; // used to avoid multiple printing same isrCounter, because loop() iterates much faster than the timer period

    // Read the global variable(s) safely, timertsk() has set them
    portENTER_CRITICAL(&timerMux_obj);
    uint32_t isrCount = IsrCounter;
    portEXIT_CRITICAL(&timerMux_obj);

    if (isrCount != Last_isr_count && isrCount % 100 == 0) {
        Last_isr_count=isrCount;
        dbprint("loop() : onTimer #");
        dbprint(isrCount);
        dbprint(" at ");
        dbprint(millis());
        dbprintln(" ms");
        
        digitalWrite(BUILTIN_LED, Led_state);
        Led_state=!Led_state;
    }
}
