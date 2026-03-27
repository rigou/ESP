/* This program is published under the GNU General Public License. 
 * This program is free software and you can redistribute it and/or modify it under the terms
 * of the GNU General  Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.
 * See the GNU General Public License for more details : https://www.gnu.org/licenses/ *GPL 
*/

#pragma once
#include <Arduino.h>

#define ANALIB_NAME     "rgAnalog"
#define ANALIB_VERSION  "v2.2.2"

class rgAnalog {

    public:
        rgAnalog(uint8_t *adc_gpios, uint8_t adc_gpios_count, uint8_t resolution=10, uint32_t conversions=20, uint32_t adc_frequency=20000);
        void Setup(void);
        void Start(void);
        void Stop(void);
        bool ReadLock();
        void ReadUnlock();

        static unsigned long TimeDiff(unsigned long ms1, unsigned long ms2);

        bool Available(void); // returns true if new results are available

        uint16_t get_Value(uint8_t idx);
        uint16_t get_Millivolt(uint8_t idx);

    private:
        // Array of gpios that will be used for ADC Continuous mode
        // ADC1 has 8 inputs, attached to GPIOs 32-39 (8 GPIOs)
        uint8_t *mAdcGpios=nullptr;
        uint8_t mAdcGpiosCount=0;

        // Set the resolution to 9-12 bits
        uint8_t mResolution;

        // Define how many conversion per pin will happen and reading the data will be and average of all conversions
        uint32_t mConversions;

        // Set sampling frequency of ADC in Hz, minimum 20 kHz
        uint32_t mAdcFrequency;

        // Array of ADC Continuous reading result structures, set by and belonging to analogContinuousRead() 
        adc_continuous_data_t *mReadBuffer=nullptr;

        static const uint8_t MAX_INPUTS=8;      // up to 8 inputs
        uint16_t mAdcResults[2][MAX_INPUTS];    // index 0=adc value, index 1=millivolt
        
        bool mAvailable=false; // true=new results are available

        SemaphoreHandle_t mReadSemaphore;

        static const size_t READADC_STACKSIZE=2000; // bytes
        
        bool mAdcRunning=false;
        
        static void async_read_adc(void *argp);
};
