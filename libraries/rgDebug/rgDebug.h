#pragma once

#define DBLIB_NAME "rgDebug"
#define DBLIB_VERSION "v1.1.0"

#define UART1_RX_GPIO 25
#define UART1_TX_GPIO 26

// Oscilloscope output on SCOPE_GPIO is handled by these macros
// Do not define SCOPE_GPIO if you don't need it
#ifdef SCOPE_GPIO
#define setupScope()	{pinMode(SCOPE_GPIO, OUTPUT); dbprintf("Scope on gpio %d\n", SCOPE_GPIO);}
#define writeScope(x)	digitalWrite(SCOPE_GPIO, (x))
#else
#define setupScope(...)
#define writeScope(...)
#endif

// All debug output is handled by these macros, Serial is never called directly for debug output
// Every module of your application must #define DEBUG_ON with the appropriate debug level
// DEBUG_ON : 0=debug off, 1=output to Serial, 2=output to serial UART1
// How to initialize UART1:
//	#include <rgDebug.h>
//	void setup() {
//	Serial.println("To watch the output of UART1, attach a serial terminal with: minicom -b 9600 -D /dev/ttyUSB1");
//	SerialPort1.begin(9600, SERIAL_8N1, UART1_RX_GPIO, UART1_TX_GPIO);
//	}
//	To watch the output of UART1, attach a serial terminal with: minicom -b 9600 -D /dev/ttyUSB1
#ifndef DEBUG_ON
#define DEBUG_ON 0 // debug off by default
#endif

#if DEBUG_ON == 2 || TRACE_ON == 2
#define DB_UART1_ENABLED
HardwareSerial SerialPort1(1);
#endif

#if DEBUG_ON == 1
#define dbprint(...)   Serial.print(__VA_ARGS__)
#define dbprintln(...) Serial.println(__VA_ARGS__)
#define dbprintf(...)  Serial.printf(__VA_ARGS__)
#elif DEBUG_ON == 2
#define dbprint(...)   SerialPort1.print(__VA_ARGS__)
#define dbprintln(...) SerialPort1.println(__VA_ARGS__)
#define dbprintf(...)  SerialPort1.printf(__VA_ARGS__)
#else
#define dbprint(...)
#define dbprintln(...)
#define dbprintf(...)
#endif

// all trace output is handled by these macros, Serial is never called directly for tracing
// Every module of your application must #define TRACE_ON with the appropriate tracing level
// TRACE_ON : 0=trace off, 1=trace to serial, 2=trace to serial UART1
// How to initialize UART1: see above
#ifndef TRACE_ON
#define TRACE_ON 0
#endif
#if TRACE_ON == 1
#define trprint(...)   Serial.print(__VA_ARGS__)
#define trprintln(...) Serial.println(__VA_ARGS__)
#define trprintf(...)  Serial.printf(__VA_ARGS__)
#elif TRACE_ON == 2
#define trprint(...)   SerialPort1.print(__VA_ARGS__)
#define trprintln(...) SerialPort1.println(__VA_ARGS__)
#define trprintf(...)  SerialPort1.printf(__VA_ARGS__)
#else
#define trprint(...)
#define trprintln(...)
#define trprintf(...)
#endif
