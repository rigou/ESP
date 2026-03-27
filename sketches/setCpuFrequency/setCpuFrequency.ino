/*
* LAB: 8
* Name: ESP32 Read Default Clocks
* Author: Khaled Magdy
* https://deepbluembedded.com/esp32-change-cpu-speed-clock-frequency/
*/
 

uint32_t NewFreq = 0; // 10,20,40,80,160,240

void setup()
{
	Serial.begin(115200);
	while (!Serial) ;
	// give user some time to open the serial monitor
	int count_int=0;
	Serial.println("");
	while (count_int<5) {
		Serial.printf("%c", 'A'+count_int++);
		delay(1000);
	}
}
 
void loop()
{
	uint32_t freq_int = 0;

	
	if (NewFreq==0 || NewFreq==240)
		NewFreq=10;
	else if (NewFreq==160)
		NewFreq=240;
	else
		NewFreq*=2;
		
	setCpuFrequencyMhz(NewFreq);
  	Serial.begin(115200); // must set serial speed after changing cpu freq
  	
	Serial.println("");
	freq_int = getCpuFrequencyMhz();
	Serial.print("CPU Freq = ");
	Serial.print(freq_int);
	Serial.println(" MHz");
	freq_int = getXtalFrequencyMhz();
	Serial.print("XTL Freq = ");
	Serial.print(freq_int);
	Serial.println(" MHz");
	freq_int = getApbFrequency();
	Serial.print("APB Freq = ");
	Serial.print(freq_int/1000000);
	Serial.println(" MHz");
	delay(5000);
}
