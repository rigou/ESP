// TimerDemo.ino - This example shows how to use a hardware timer in ESP32
// 2024-05-31, 2026-04-18
// derived from the example at https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html

#include <Arduino.h>

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

// Stop button
#define BTN_STOP_ALARM 33 // gpio of the momentary push button

hw_timer_t *Timer_obj = NULL;
volatile SemaphoreHandle_t Semaphore_obj;
portMUX_TYPE timerMux_obj = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

void ARDUINO_ISR_ATTR onTimer() {
	
  // Increment the counter and set the time of ISR, accessing volatile global variables safely
  portENTER_CRITICAL_ISR(&timerMux_obj);
  isrCounter = isrCounter + 1;
  lastIsrAt = millis();
  portEXIT_CRITICAL_ISR(&timerMux_obj);
	
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(Semaphore_obj, NULL);
	
  // Notice: it is safe to use digitalRead/Write here if you want to toggle an output
}

void setup() {
  Serial.begin(921600);
  while (!Serial);
	dbprint('\n'); for (uint8_t idx=0; idx<13; idx++) { dbprint((char)('A'+idx)); delay(500); }
  dbprint('\n');
  dbprintln("Setup");

  pinMode(BTN_STOP_ALARM, INPUT_PULLUP);
	
  // Create semaphore to inform us when the timer has fired
  Semaphore_obj = xSemaphoreCreateBinary();

  // Set timer frequency to 1 MHz to get a 1 microsecond resolution
  const uint32_t TIMER_FREQ = 1000000;
  Timer_obj = timerBegin(TIMER_FREQ);

  // Attach onTimer function to our timer
  timerAttachInterrupt(Timer_obj, &onTimer);

  // Set alarm to call onTimer function 10 times/second
  // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter)
  // onTimer() will be called when the counter of Timer_obj reaches this value, and the counter will be reset to 0
  timerAlarm(Timer_obj, TIMER_FREQ/10, true, 0);

  dbprintln("Setup complete");
}

void loop() {
  // if Timer has fired then the semaphore is released and xSemaphoreTake() succeeds
  // parameter #2 is xTicksToWait : the time in ticks to wait for the semaphore to become available. 
  // The macro portTICK_PERIOD_MS can be used to convert this to a real time. 
  // A block time of zero can be used to poll the semaphore. 
  // Specifying the block time as portMAX_DELAY will cause the task to block indefinitely (without a timeout).
  if (xSemaphoreTake(Semaphore_obj, portMAX_DELAY) == pdTRUE) {
    uint32_t isrCount = 0;
    uint32_t isrTime = 0;
	  
    // Read the interrupt count and time, accessing volatile global variables safely
    portENTER_CRITICAL(&timerMux_obj);
    isrCount = isrCounter;
    isrTime = lastIsrAt;
    portEXIT_CRITICAL(&timerMux_obj);
	  
    // Print it
    dbprint("onTimer #");
    dbprint(isrCount);
    dbprint(" at ");
    dbprint(isrTime);
    dbprintln(" ms");
  }
  
  if (digitalRead(BTN_STOP_ALARM) == LOW) {
    // If timer is still running
    if (Timer_obj) {
      // Stop and free timer
      timerEnd(Timer_obj);
      Timer_obj = NULL;
      dbprintln("Stopped.");
    }
  }
}
