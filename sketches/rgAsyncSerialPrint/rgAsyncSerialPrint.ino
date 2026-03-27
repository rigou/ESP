// rgAsyncSerialPrint.ino
#include <Arduino.h>

#define DEBUG_ON 1
#include <rgDebug.h>

#define LED_GPIO 2 

// FreeRTOS queue used to communicate from loop() to task_read_msg
static QueueHandle_t Qh;
#define QREAD_STACK_SIZE 8192
#define QTEXTLEN 10
typedef struct ReadQueueData {
    uint16_t line;
    uint16_t count;
    uint8_t unused_alignment;
    char text[QTEXTLEN+1];
} ReadQData; // 16 bytes

/* Timing accuracy in loop() is obtained by running task_read_msg() in core 0 while loop() runs in core 1 (default app core)
*
task_read_msg() running in core 0 :
    16:55:14.549 -> rx line 1 count 101 stack 9660 time 37 ms
    16:55:15.579 -> rx line 1 count 201 stack 8784 time 37 ms
    16:55:16.575 -> rx line 1 count 301 stack 8784 time 37 ms
    16:55:17.571 -> rx line 1 count 401 stack 8784 time 37 ms
    16:55:18.567 -> rx line 1 count 501 stack 8784 time 37 ms
    16:55:19.564 -> rx line 1 count 601 stack 8784 time 37 ms
    16:55:20.560 -> rx line 1 count 701 stack 8784 time 37 ms
    16:55:21.557 -> rx line 1 count 801 stack 8784 time 37 ms
    16:55:22.553 -> rx line 1 count 901 stack 8784 time 37 ms
    16:55:23.550 -> rx line 1 count 1001 stack 8784 time 37 ms
    16:55:24.546 -> rx line 1 count 1101 stack 8784 time 37 ms
    16:55:25.576 -> rx line 1 count 1201 stack 8784 time 37 ms
    16:55:26.572 -> rx line 1 count 1301 stack 8784 time 37 ms
    16:55:27.569 -> rx line 1 count 1401 stack 8784 time 37 ms
    16:55:28.565 -> rx line 1 count 1501 stack 8784 time 37 ms
    16:55:29.561 -> rx line 1 count 1601 stack 8784 time 37 ms
    16:55:30.558 -> rx line 1 count 1701 stack 8784 time 37 ms
    16:55:31.554 -> rx line 1 count 1801 stack 8784 time 37 ms
    16:55:32.550 -> rx line 1 count 1901 stack 8784 time 37 ms
    16:55:33.546 -> rx line 1 count 2001 stack 8784 time 37 ms
*
task_read_msg() running in core 1 :
    16:58:55.646 -> rx line 1 count 101 stack 9660 time 37 ms
    16:58:56.641 -> rx line 1 count 201 stack 8556 time 37 ms
    16:58:57.637 -> rx line 1 count 301 stack 8556 time 37 ms
    16:58:58.666 -> rx line 1 count 401 stack 8556 time 37 ms
    16:58:59.662 -> rx line 1 count 501 stack 8556 time 37 ms
    16:59:00.659 -> rx line 1 count 601 stack 8556 time 37 ms
    16:59:01.655 -> rx line 1 count 701 stack 8556 time 37 ms
    16:59:02.651 -> rx line 1 count 801 stack 8556 time 37 ms
    16:59:03.648 -> rx line 1 count 901 stack 8556 time 37 ms
    16:59:04.644 -> rx line 1 count 1001 stack 8556 time 37 ms
    16:59:05.641 -> rx line 1 count 1101 stack 8556 time 37 ms
    16:59:06.670 -> rx line 1 count 1202 stack 8556 time 37 ms
    16:59:07.666 -> rx line 1 count 1302 stack 8556 time 37 ms
    16:59:08.663 -> rx line 1 count 1403 stack 8556 time 37 ms
    16:59:09.659 -> rx line 1 count 1503 stack 8556 time 37 ms
    16:59:10.688 -> rx line 1 count 1604 stack 8556 time 37 ms
    16:59:11.684 -> rx line 1 count 1704 stack 8556 time 37 ms
    16:59:12.679 -> rx line 1 count 1805 stack 8556 time 37 ms
    16:59:13.676 -> rx line 1 count 1905 stack 8556 time 37 ms
    16:59:14.705 -> rx line 1 count 2006 stack 8556 time 37 ms
*/

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
    // setting a higher loopTask priority is ok if loop() yields with delay(1)
    // but noy with taskYIELD(). WHY ?
    //vTaskPrioritySet(nullptr,2);

    Serial.begin(115200);
	while (!Serial) ; // wait for serial port to connect
	dbprint('\n'); for (uint8_t idx = 0; idx<3; idx++) { dbprint((char)('A'+idx)); delay(500); }
	dbprint('\n'); 
    dbprintf("ReadQData %d bytes\n", sizeof(ReadQData));

    TaskHandle_t th;
    BaseType_t rto_status;

    Qh = xQueueCreate(1, sizeof(ReadQData)); // depth=1
    assert(Qh);

    pinMode(LED_GPIO,OUTPUT);
    
    // task task_read_msg() runs on core 0 with priority 1
    rto_status = xTaskCreatePinnedToCore(
        task_read_msg,
        "qread",
        QREAD_STACK_SIZE,
        nullptr,  // Not used
        1,        // same priority as loopTask
        &th,      // Task handle
        0         // Run on core 0
    );
    assert(rto_status == pdPASS);
    assert(th);

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
    
    if (xSemaphoreTake(Semaphore_obj, 0) == pdTRUE) {
        // data processing takes place here
        static unsigned int Counter=0;
        Counter++;
        const char *LINES[4]={"The quick", "brown fox", "jumps over", "the dog"};

        // display app data periodically and asynchronously
        if (Counter % 100 == 0) {
            ReadQData qdata;
            uint16_t idx=(Counter/100) % 4; // 0 1 2 3
            qdata.line=idx+1;               // 1 2 3 4
            memset(qdata.text, 0, QTEXTLEN+1);
            strncpy(qdata.text, LINES[idx], QTEXTLEN);
            qdata.count=Counter;

            // xQueueSendToBack() takes 19 microseconds at 80 MHz, 7 microseconds at 240 MHz
            // unsigned long qcall_timer=micros();
            if (xQueueSendToBack(Qh,&qdata,sizeof(ReadQData)) != pdPASS)
                Serial.println("full");
            /*
            else {
                qcall_timer=micros()-qcall_timer;
                Serial.printf("tx line %u count %u in %lu micros\n", qdata.line, qdata.count, qcall_timer);
            }
            */
        }
    }
}

// Queue receiving task, blocks reading the queue
static void task_read_msg(void *argp) {
    BaseType_t rto_status;
    unsigned stack_now = 0;
    unsigned stack_max = 0; // used stack maximum (bytes)
    ReadQData qdata;

    while (1) {
        rto_status = xQueueReceive(
            Qh,
            &qdata,
            portMAX_DELAY
        );
        assert(rto_status == pdPASS);
        unsigned long processing_time=millis();
        digitalWrite(LED_GPIO,(qdata.count/100)%2==1);

        stack_now=QREAD_STACK_SIZE - uxTaskGetStackHighWaterMark(nullptr);
        if (stack_now > stack_max )
            stack_max = stack_now;

        // simulate app processing : 35 ms >= exec time of rgDisplay::PrintLine()
        unsigned long timer=millis();
        while (millis()<timer+35);
        Serial.printf("rx line %u count %u stack %u time %lu ms text %s\n", qdata.line, qdata.count, stack_now, millis()-processing_time, qdata.text);
        //Serial.printf("rx line %u count %u stack %u time %lu ms\n", qdata.line, qdata.count, stack_now, millis()-processing_time);
    }
}
