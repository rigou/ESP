/* rgAnalog.cpp - Continuous and asynchronous ADC sampling 
	This class reads the ADC continously on 2-Cores ESP32 microcontrollers
  	It is designed to run in Core 1, except method async_read_adc() which runs on Core 0
    Using this class, you can read the ADC without waiting for analogContinuousRead() to complete,
    this is handy in cases where this wait would disrupt the timing of taskLoop.
    See usage example rgAnalogTest.ino
*/

/* This program is published under the GNU General Public License. 
 * This program is free software and you can redistribute it and/or modify it under the terms
 * of the GNU General  Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.
 * See the GNU General Public License for more details : https://www.gnu.org/licenses/ *GPL 
*/

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc_continuous.html
// https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html

// About ADC usage:
// ADC1 has 8 inputs, attached to GPIOs 32-39. 
//  GPIOs 34-39 are input-only, and they don't have internal pull-up resistors
//  and as such cannot be used to digitalRead the state of a switch
// ADC2 has 10 inputs, attached to GPIOs 0, 2, 4, 12-15 and 25-27
//  ADC2 is used by the Wi-Fi driver : you can only use ADC2 when the Wi-Fi driver has not been started.
//  	ESP32 DevKitC: GPIO 0 cannot be used due to external auto program circuits.
//		ESP-WROVER-KIT: GPIO 0, 2, 4, and 15 cannot be used due to external connections for different purposes.

#include "rgAnalog.h"

// 0=debug off, 1=output to Serial, 2=output to serial UART1
#define DEBUG_ON 0
// 0=trace off, 1=output to serial, 2=output to serial UART1
#define TRACE_ON 0
#include <rgDebug.h>

// adc_gpios : array of the GPIO numbers used as ADC inputs (32-39 on ADC1)
// adc_gpios_count : number of items in array adc_gpios
// resolution : 9-12 (bits)
// conversions : how many conversions per gpio will run each ADC cycle
// adc_frequency : sampling frequency of ADC in Hz (default 20 kHz). Setting a higher frequency does not improve rgAnalog::Read() execution time
rgAnalog::rgAnalog(
        uint8_t *adc_gpios,
        uint8_t adc_gpios_count,
        uint8_t resolution,
        uint32_t conversions,
        uint32_t adc_frequency
    ) {
    mAdcGpios=adc_gpios;
    if (adc_gpios_count>MAX_INPUTS)
        adc_gpios_count=MAX_INPUTS;
    mAdcGpiosCount=adc_gpios_count;
    
    // Set the resolution to 9-12 bits
    mResolution=resolution;
    // Define how many conversion per pin will happen and reading the data will be and average of all conversions
    mConversions=conversions;
    // Set sampling frequency of ADC in Hz, minimum 20 kHz
    mAdcFrequency=adc_frequency;
}

void rgAnalog::Setup(void) {
    trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    // Optional for ESP32: Set the resolution to 9-12 bits (default is 12 bits)
    analogContinuousSetWidth(mResolution);

    // Optional: Set different attenuation (default is ADC_11db)
    analogContinuousSetAtten(ADC_11db);

    // Setup ADC Continuous with following input:
    // array of pins, count of the pins, how many conversions per pin in one cycle will happen, sampling frequency, callback function
    analogContinuous(mAdcGpios, mAdcGpiosCount, mConversions, mAdcFrequency, nullptr);
    this->Start();
    
    // This 2-dimensional array contains a copy of the contents of mReadBuffer[]
    // this copy is made by async_read_adc(), accessed by get_Value() and get_Millivolt()
    // mAdcResults[2][mAdcGpiosCount] index 0=adc value, index 1=millivolt
    /*
    trprintf("calloc %u x %d bytes\n", 2*mAdcGpiosCount, sizeof(uint16_t));
    mAdcResults=(uint16_t **)calloc(2*mAdcGpiosCount, sizeof(uint16_t));
    */
    // create semaphore
    mReadSemaphore = xSemaphoreCreateBinary();
    assert(mReadSemaphore);
    BaseType_t rto_status = xSemaphoreGive(mReadSemaphore);
    assert(rto_status == pdPASS);
    // create task async_read_adc() on core 0 with priority 1
    TaskHandle_t th;
    rto_status = xTaskCreatePinnedToCore(
        async_read_adc,
        "readadc",
        READADC_STACKSIZE,
        this,
        1,        // same priority as loopTask
        &th,      // Task handle
        0         // Run on core 0
    );
    assert(rto_status == pdPASS);
    assert(th);
    trprintf("*** %s %s() end\n", __FILE_NAME__, __FUNCTION__);
}

void rgAnalog::Start(void) {
    trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    if (!mAdcRunning)
        mAdcRunning=analogContinuousStart();
    trprintf("*** %s %s() end\n", __FILE_NAME__, __FUNCTION__);
}

void rgAnalog::Stop(void) {
    trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    if (mAdcRunning)
        mAdcRunning=!analogContinuousStop();
    trprintf("*** %s %s() end\n", __FILE_NAME__, __FUNCTION__);
}

// Take the semaphore prior to calling get_Value() or get_Millivolt()
// return value: true=success, false=semaphore not available
bool rgAnalog::ReadLock() {
    trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    BaseType_t rto_status = xSemaphoreTake(mReadSemaphore, 0); // 0 = do not retry if not available
    trprintf("*** %s %s() returns %d\n", __FILE_NAME__, __FUNCTION__, rto_status == pdPASS);
    return (rto_status == pdPASS);
}

// Give the semaphore after calling get_Value() or get_Millivolt()
void rgAnalog::ReadUnlock() {
    trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    mAvailable=false;
    BaseType_t rto_status = xSemaphoreGive(mReadSemaphore);
    trprintf("*** %s %s() returns %d\n", __FILE_NAME__, __FUNCTION__, rto_status == pdPASS);
    assert(rto_status == pdPASS);
}

// Test if new results are available
// must call ReadLock() before calling this method
bool rgAnalog::Available(void) {
    trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    trprintf("*** %s %s() returns %d\n", __FILE_NAME__, __FUNCTION__, mAvailable);
    return mAvailable;
}

// must call ReadLock() before calling this method
uint16_t rgAnalog::get_Value(uint8_t idx) {
    trprintf("*** %s %s(%u) begin\n", __FILE_NAME__, __FUNCTION__, idx);
    assert(idx<mAdcGpiosCount);
    trprintf("*** %s %s(%u) returns %d\n", __FILE_NAME__, __FUNCTION__, idx, mAdcResults[0][idx]);
    return mAdcResults[0][idx];
}

// must call ReadLock() before calling this method
uint16_t rgAnalog::get_Millivolt(uint8_t idx) {
    trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    assert(idx<mAdcGpiosCount);
    trprintf("*** %s %s(%u) returns %d\n", __FILE_NAME__, __FUNCTION__, idx, mAdcResults[1][idx]);
    return mAdcResults[1][idx];
}

// This ADC reading loop continuously reads conversion results and copies them in mAdcResults[][]
// Stack usage: 1652 bytes
void rgAnalog::async_read_adc(void *argp) {
    trprintf("*** %s %s() begin\n", __FILE_NAME__, __FUNCTION__);
    assert(argp);
	rgAnalog *analog_reader_obj=(rgAnalog *)argp; // arg 3 of xTaskCreatePinnedToCore() in Setup()

    while (1) {
        if (analog_reader_obj->mAdcRunning) {
            // store latest conversion results in mReadBuffer, this may take several milliseconds
            if (analogContinuousRead(&(analog_reader_obj->mReadBuffer), 0)) {
                // take semaphore, retry for 5 seconds if not immediately available
                BaseType_t rto_status = xSemaphoreTake(analog_reader_obj->mReadSemaphore, pdMS_TO_TICKS(5000));
                if (rto_status == pdPASS) {
                    // copy mReadBuffer into mAdcResults
                    for (int idx=0; idx<analog_reader_obj->mAdcGpiosCount; idx++) {
                        analog_reader_obj->mAdcResults[0][idx]=analog_reader_obj->mReadBuffer[idx].avg_read_raw;
                        analog_reader_obj->mAdcResults[1][idx]=analog_reader_obj->mReadBuffer[idx].avg_read_mvolts;
                    }
                    // give semaphore
                    rto_status = xSemaphoreGive(analog_reader_obj->mReadSemaphore);
                    assert(rto_status == pdPASS);
                    analog_reader_obj->mAvailable=true; // new results are available

                    /*/ display stack usage
                    static char stack_str[12];
                    unsigned stack_used=READADC_STACKSIZE - uxTaskGetStackHighWaterMark(nullptr);
                    snprintf(stack_str, 11, "%u/%u", stack_used, READADC_STACKSIZE);
                    Serial.println(stack_str);
                    */
                }
            }
        }

        /* allow loopTask() to take the semaphore
        Without this delay, for mAdcGpiosCount > 4, the program crashes with:
        08:26:44.511 -> E (13755) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
        08:26:44.545 -> E (13755) task_wdt:  - IDLE0 (CPU 0)
        08:26:44.545 -> E (13755) task_wdt: Tasks currently running:
        08:26:44.545 -> E (13755) task_wdt: CPU 0: readadc
        08:26:44.545 -> E (13755) task_wdt: CPU 1: IDLE1
        08:26:44.545 -> E (13755) task_wdt: Aborting.
        */
        delay(5); 
    }
}
