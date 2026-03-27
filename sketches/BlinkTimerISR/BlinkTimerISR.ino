// BlinkTimerISR.ino - Blink implemented with autorepeat timer
// derived from Examples/ESP32/Timer/RepeatTimer (which shows also how to use critical sections)
// Compilation instructions: cd 
//  cd ~/Projects/Espressif
//  ./bwacli.sh BlinkTimerISR -u ttyUSB0 -f 80
// 2023-07-30

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

hw_timer_t * Timer_obj = NULL;
volatile SemaphoreHandle_t Semaphore_obj;

void ARDUINO_ISR_ATTR onTimer(){
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(Semaphore_obj, NULL);
}

void setup() {
    // this semaphore tells when the timer has fired
    // we use it to make the onTimer() ISR return as fast as possible
    Semaphore_obj = xSemaphoreCreateBinary();
    // Set timer frequency to 10 kHz to get a 100 microsecond resolution (slower frequency won't work)
    Timer_obj = timerBegin(10000);
    // Attach onTimer function to our timer.
    timerAttachInterrupt(Timer_obj, &onTimer);
    // Set alarm to call onTimer function every 500000 microseconds
    // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter)
    // onTimer() will be called when the counter of Timer_obj reaches this value, and the counter will be reset to 0
    timerAlarm(Timer_obj, 500000/100, true, 0);


  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  static unsigned long count=0;
  if (xSemaphoreTake(Semaphore_obj, 0) == pdTRUE)
    digitalWrite(LED_BUILTIN, ++count%2==0); 
}