#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Arduino.h>
#define REPORTING_PERIOD_MS     100
#include "MAX30100.h"

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation


PulseOximeter pox;

uint32_t tsLastReport = 0;

void setup()
{
	Serial.begin(115200);

	Serial.print("Initializing pulse oximeter..");

	if (!pox.begin()) {
		Serial.println("FAILED");
		for (;;);
	}
	else {
		Serial.println("SUCCESS");
	}

}

void loop()
{
	// Make sure to call update as fast as possible
	pox.update();

	// Asynchronously dump heart rate and oxidation levels to the serial
	// For both, a value of 0 means "invalid"


	if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
		float hr = pox.getHeartRate();
		float spo2 = pox.getSpO2();

		String message = "{\"heartrate\":" + String(hr) + ",\"spo2\":" + String(spo2) + "}";
		Serial.println(message);

		tsLastReport = millis();


	}
}
