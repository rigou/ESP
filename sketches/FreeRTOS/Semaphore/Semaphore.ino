// ESP32 Binary Semaphore Example
// https://www.circuitstate.com/tutorials/how-to-write-parallel-multitasking-applications-for-esp32-using-freertos-arduino/

#include <Arduino.h>

// 0=debug off, 1=output to Serial, 2=output to serial UART1()
#define DEBUG_ON 1
// 0=trace off, 1=output to Serial, 2=output to serial UART1()
#define TRACE_ON 0
#include <rgDebug.h>

#ifndef BUILTIN_LED
#define BUILTIN_LED 2
#endif

SemaphoreHandle_t xSemaphore = NULL;  // Create a semaphore object

//================================================================================//

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(921600);
    while (!Serial);
#if DEBUG_ON
	dbprint('\n'); for (uint8_t idx=0; idx<13; idx++) { dbprint((char)('A'+idx)); delay(500); }
#endif
	dbprint('\n');
	
  // initialize digital pin BUILTIN_LED as an output.
  pinMode (BUILTIN_LED, OUTPUT);

  xSemaphore = xSemaphoreCreateBinary();  // Set the semaphore as binary

  xTaskCreatePinnedToCore (
    task1,     // Function to implement the task
    "task1",   // Name of the task
    1000,      // Stack size in words
    NULL,      // Task input parameter
    10,         // Priority of the task
    NULL,      // Task handle.
    0          // Core where the task should run
  );

  xTaskCreatePinnedToCore (
    task2,     // Function to implement the task
    "task2",   // Name of the task
    1000,      // Stack size in words
    NULL,      // Task input parameter
    10,         // Priority of the task
    NULL,      // Task handle.
    0          // Core where the task should run
  );
}

//================================================================================//

// the loop function runs over and over again forever
void loop() {
  digitalWrite (BUILTIN_LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay (250);  // wait
  digitalWrite (BUILTIN_LED, LOW); // turn the LED off by making the voltage LOW
  delay (250);  // wait
}

//================================================================================//

// this task will periodically release the binary semaphore
void task1 (void *pvParameters) {
  while (1) {
    Serial.print ("Binary Semaphore released at ");
    Serial.println (xTaskGetTickCount());
    xSemaphoreGive (xSemaphore);  // Release the semaphore
    delay (1000);
  }
}

//================================================================================//

// this task will wait for the binary semaphore to be released
void task2 (void *pvParameters) {
  while (1) {
    if (xSemaphoreTake (xSemaphore, (200 * portTICK_PERIOD_MS))) {  // try to acquire the semaphore
      Serial.print ("Binary Semaphore acquired at ");
      Serial.println (xTaskGetTickCount());
    }
    else {  // if the semaphore was not acquired within 200ms
      Serial.print ("Binary Semaphore not acquired at ");
      Serial.println (xTaskGetTickCount());
    }
  }
}
