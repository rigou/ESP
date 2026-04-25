// TimerThreadMod.ino - This example shows how to use a hardware timer on a separate task in ESP32,
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

void setup() {
  Serial.begin(921600);
  while (!Serial);
	dbprint('\n'); for (uint8_t idx=0; idx<13; idx++) { dbprint((char)('A'+idx)); delay(500); }
  dbprint('\n');

  pinMode(BUILTIN_LED, OUTPUT);
  ttm_Setup();
  dbprintln("setup complete");
}

// loops processes actions that are not time-critical
void loop() {
  digitalWrite(BUILTIN_LED, HIGH);
  delay(250);  // wait
  digitalWrite(BUILTIN_LED, LOW);
  delay(250);  // wait
}
