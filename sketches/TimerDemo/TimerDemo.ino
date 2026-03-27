// TimerDemo.ino - This example shows how to use a hardware timer in ESP32
// 2024-05-31
// source: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html

hw_timer_t *Timer_obj = NULL;
volatile SemaphoreHandle_t Semaphore_obj;
portMUX_TYPE timerMux_obj = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

// Stop button
#define BTN_STOP_ALARM 16

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
  Serial.begin(115200);
  Serial.println("Setup");

  pinMode(BTN_STOP_ALARM, INPUT_PULLUP);
	
  // Create semaphore to inform us when the timer has fired
  Semaphore_obj = xSemaphoreCreateBinary();

  // Set timer frequency to 10 kHz to get a 100 microsecond resolution
  Timer_obj = timerBegin(10000);

  // Attach onTimer function to our timer
  timerAttachInterrupt(Timer_obj, &onTimer);

  // Set alarm to call onTimer function every 1/10 second
  // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter)
  // onTimer() will be called when the counter of Timer_obj reaches this value, and the counter will be reset to 0
  timerAlarm(Timer_obj, 1000, true, 0);

  Serial.println("Setup complete");
}

void loop() {
  // If Timer has fired
  if (xSemaphoreTake(Semaphore_obj, 0) == pdTRUE) {
    uint32_t isrCount = 0, isrTime = 0;
	  
    // Read the interrupt count and time, accessing volatile global variables safely
    portENTER_CRITICAL(&timerMux_obj);
    isrCount = isrCounter;
    isrTime = lastIsrAt;
    portEXIT_CRITICAL(&timerMux_obj);
	  
    // Print it
    Serial.print("onTimer no. ");
    Serial.print(isrCount);
    Serial.print(" at ");
    Serial.print(isrTime);
    Serial.println(" ms");
  }
  
  if (digitalRead(BTN_STOP_ALARM) == LOW) {
    // If timer is still running
    if (Timer_obj) {
      // Stop and free timer
      timerEnd(Timer_obj);
      Timer_obj = NULL;
      Serial.println("Stopped.");
    }
  }
}
