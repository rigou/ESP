const int SERIAL_BAUDRATE = 115200;

const int VALUES_COUNT=100;
byte Values[VALUES_COUNT];
    
void loop() {
}

void setup() {
   	Serial.begin(SERIAL_BAUDRATE);
	while (!Serial) ; // wait for serial port to connect
	Serial.println("");

	// give user some time to open the serial monitor
	for (int idx=0; idx < 12; idx++) {
		Serial.print((char)('A'+idx));
		delay(500);
	}
	Serial.println("");

    ArrangeValues(4294967295, 83, Values, VALUES_COUNT);
   	for (int idx=0; idx<VALUES_COUNT; idx++)
        Serial.printf("%02d\n",Values[idx]);
}

// Fill given array with distinct random values
// if values_out_count <= max_value there will be not duplicate values in the array
// else the number duplicates will be minimal
// key is used to seed the RNG
// max_value the array will be filled with values in range (0, max_value)
// values_out is the resulting array
// values_out_count is the number of random values stored in the resulting array
void ArrangeValues(const unsigned int key, const byte max_value, byte *values_out, const byte values_out_count) {
    const byte NO_VALUE=255;
    bool available_values[values_out_count];
    int idx=0;
	for (idx=0; idx<values_out_count; idx++) {
        values_out[idx]=NO_VALUE;
        available_values[idx]=true;
    }
	randomSeed(key);
    for (idx=0; idx<values_out_count; idx++) {
        while (values_out[idx]==NO_VALUE) {
            byte idx_rnd=random(values_out_count); // 0-99
            if (available_values[idx_rnd]) {
                available_values[idx_rnd]=false;
                if (idx_rnd<=max_value)
                    values_out[idx]=idx_rnd;
                else
                    values_out[idx]=(idx_rnd%max_value)-1;
            }
        }
    }
}
