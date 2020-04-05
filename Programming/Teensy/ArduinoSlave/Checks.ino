
void setBreathsPerMinuteParam(int numberOfBreaths) {

	int validNBreaths = hardLimiter(minBreathsPM, maxBreathsPM, numberOfBreaths);

	if (validNBreaths != -1) {
		conf.breathsPerMinute = validNBreaths;
		HWSERIAL.println("BreathesPM OK");
	}



}

void setMotionLengthParam(long mLength) {

	long validMLength = hardLimiter(minMotionLength, maxMotionLength, mLength);

	if (validMLength != -1) {
		conf.motionLength = validMLength;
		HWSERIAL.println("MotionLength OK");
	}

}

void setInhaleExhaleRateParam(int rate) {

	int validRate = hardLimiter(minInhaleRatio, maxInhaleRatio, rate);

	if (validRate != -1) {
		conf.inhaleExhaleRatio = validRate;
		HWSERIAL.println("IE Ratio OK");
	}

}


void setAirVolParam(int rate) {

	int validRate = hardLimiter(minAirVol, maxAirVol, rate);

	if (validRate != -1) {
		conf.airVol = validRate;
		HWSERIAL.println("AirVolume Param OK");
	}

}


void homeSwitchCheck() {

	unsigned long beforePressingTime = millis();


	while (!digitalRead(ZERO_POS)) {
		
		unsigned long currentMillis = millis();

		if (currentMillis - beforePressingTime > 100) {

			//Serial.println("This is a valid press");

			zeroed = false;
			//Serial.println("Switch Pressed");
			break;
		}

	}

}
