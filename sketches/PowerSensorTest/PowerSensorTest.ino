/* PowerSensorTest.ino
 * Copyright Richard Goutorbe 2024-06-06
 */

#include "PowerSensor.h"

const byte POWERSENSOR_GPIO=36;
const float SENSOR_CALIBRATION = 4614.0 / 1728.0; // 4614 mV sampled as 1728 mV

// ADC input at POWERSENSOR_GPIO, compute 5 averaged values/s, with 10 bit resolution
PowerSensor PowerSensor_obj(POWERSENSOR_GPIO, 5, 10);

void setup() {
    Serial.begin(115200);
    Serial.print('\n'); for (int idx = 0; idx<4; idx++) { Serial.print((char)('A'+idx)); delay(500); }; Serial.print('\n');
    PowerSensor_obj.Config();
}

void loop() {
    uint16_t avg_millivolts;
    if (PowerSensor_obj.ReadVoltage(SENSOR_CALIBRATION, &avg_millivolts))
        Serial.println(avg_millivolts);
}