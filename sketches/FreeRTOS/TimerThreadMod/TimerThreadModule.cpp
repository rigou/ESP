// TimerThreadModule.cpp - All time critical code is in this module
// 2026-04-19
// derived from the example at https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html

// a more efficient implementation is described here:
// https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/03-Direct-to-task-notifications/02-As-binary-semaphore

#include <Arduino.h>

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0

// You can use an oscilloscope to observe these events (macro defined in rgDebug.h) - define SCOPE_GPIO if you need this
#define SCOPE_GPIO 21 
#include <rgDebug.h>

#ifndef BUILTIN_LED
#define BUILTIN_LED 2
#endif

// Hardware timer
hw_timer_t *Timer_obj = NULL;

// Binary semaphore  used to synchronize task "timertsk" with the timer events
volatile SemaphoreHandle_t Semaphore_obj;
portMUX_TYPE timerMux_obj = portMUX_INITIALIZER_UNLOCKED;

// onTimer() sets these global variables and timertsk() reads them
volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

// forward declarations
void ARDUINO_ISR_ATTR onTimer();
void timertsk(void *pvParameters);
void process_alarm(void);

void ttm_Setup(void) {
  setupScope();

  xTaskCreatePinnedToCore (
    timertsk,   // Function to implement the task
    "timertsk", // Name of the task
    1000,       // Stack size in words ; printf() would need a large stack (4k)
    NULL,       // Task input parameter
    2,          // give the task a higher priority than the main loop (priority 1)
    NULL,       // Task handle.
    0           // Core where the task should run
  );
  
  // Create semaphore to inform us when the timer has fired
  // The semaphore is created in the 'empty' state, meaning the semaphore
  // must first be given before it can subsequently be taken
  Semaphore_obj = xSemaphoreCreateBinary();
  xSemaphoreGive(Semaphore_obj);
   
  // Set timer frequency to 1 MHz to get a 1 microsecond resolution
  const uint32_t TIMER_FREQ = 1000000;
  Timer_obj = timerBegin(TIMER_FREQ);

  // Attach onTimer function to our timer
  timerAttachInterrupt(Timer_obj, &onTimer);

  // Set alarm to call onTimer function 100 times/second
  // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter)
  // onTimer() will be called when the counter of Timer_obj reaches this value, and the counter will be reset to 0
  timerAlarm(Timer_obj, TIMER_FREQ/100, true, 0);

  dbprintln("ttm_Setup complete");
}

void ARDUINO_ISR_ATTR onTimer() {
  // Increment the counter and set the time of ISR, accessing volatile global variables safely
  portENTER_CRITICAL_ISR(&timerMux_obj);
  isrCounter = isrCounter + 1;
  lastIsrAt = millis();
  portEXIT_CRITICAL_ISR(&timerMux_obj);
  xSemaphoreGiveFromISR(Semaphore_obj, NULL); // timertsk() will take it
}

// this task will wait for the binary semaphore to be released, lock it, increment the counter by 1 and release it
void timertsk(void *pvParameters) {
  while (1) {
    process_alarm();
  }
}

// this function will wait for the binary semaphore to be released, lock it, increment the counter by 1 and release it
void process_alarm(void) {
  // if Timer has fired then the semaphore is released and xSemaphoreTake() succeeds
  // parameter #2 is xTicksToWait : the time in ticks to wait for the semaphore to become available. 
  // The macro portTICK_PERIOD_MS can be used to convert this to a real time. 
  // A block time of zero can be used to poll the semaphore. 
  // Specifying the block time as portMAX_DELAY will cause the task to block indefinitely (without a timeout).
  if (xSemaphoreTake(Semaphore_obj, portMAX_DELAY) == pdTRUE) {
#ifdef SCOPE_GPIO
    writeScope(HIGH);
    delayMicroseconds(1000);
    writeScope(LOW);
#endif

    uint32_t isrCount = 0;
    uint32_t isrTime = 0;
    
    // Read the interrupt count and time, accessing volatile global variables safely
    portENTER_CRITICAL(&timerMux_obj);
    isrCount = isrCounter;
    isrTime = lastIsrAt;
    portEXIT_CRITICAL(&timerMux_obj);
    
    // Print it
    if (isrCount % 100 == 0) {
      dbprint("onTimer #");
      dbprint(isrCount);
      dbprint(" at ");
      dbprint(isrTime);
      dbprintln(" ms");
    }
    
    // Flash Led
    if (isrCount % 50 == 0) {
      static bool Led_state=true;
      digitalWrite(BUILTIN_LED, Led_state);
      Led_state=!Led_state;
    }

  }
}
