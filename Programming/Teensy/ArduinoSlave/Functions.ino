long hardLimiter(long min, long max, long value) {

	if (value > max) {
		HWSERIAL.println("Value Out Of Range");
		return -1;
	}

	if (value < min) {
		HWSERIAL.println("Value Out Of Range");
		return -1;
	}

	return value;
}


void loadOrwriteDefaults() {
  int canWriteDefaults;
  //EEPROM_readAnything(80, canWriteDefaults);
  canWriteDefaults = -1;
  if (canWriteDefaults == -1) {
    conf.breathsPerMinute = 12;
    conf.airVol = 600;
    conf.inhaleExhaleRatio = 1;
    conf.motionLength = 10200;
    conf.stepperSpeed = 1000;
    EEPROM_writeAnything(0, conf);
    EEPROM_writeAnything(80, 1);
  }

  if (canWriteDefaults == 1) {
    EEPROM_readAnything(0, conf);
  }
}


void zero_position() {

	Serial.println("Eneted zero");
	motor.setMaxSpeed(1000);
	//motor.setAcceleration(2000);

	motor.setTargetRel(-2000000);
	
	controller.moveAsync(motor);

	Serial.println("Enetered zero 1");

	while (!switchPressed())
	{
		Serial.println("Homing");
	}

	controller.stop();

	motor.setTargetAbs(offSwitchPos);
	controller.moveAsync(motor);

	while (digitalRead(ZERO_POS))
	{
		Serial.println("Getting of switch");
	}

	//controller.stop();
	motor.setPosition(0);
	motor.setTargetAbs(1000);
	controller.moveAsync(motor);
	//Serial.println("Eneted zero 2");
	//Serial.println("Zeroed");

	ventPos = 0;
	zeroed = 1;

	//Serial.println("Eneted zero 3");
	motor.setMaxSpeed(conf.stepperSpeed);
	motor.setAcceleration(acceleration);

}


float getInhaleTime(int BPM) {

	float timeToBreathe = 60.0 / BPM;
	//Serial.println("timeToBreathe: " + String(timeToBreathe));
	float inhaleTime;

	if (conf.inhaleExhaleRatio == 1) {
		inhaleTime = timeToBreathe / 2.0f;
		//Serial.println("timeToInhale 1:1 " + String(inhaleTime));
		return inhaleTime * 1000; // returning time in ms
	}

	if (conf.inhaleExhaleRatio == 2) {
		inhaleTime = timeToBreathe / 3.0f;
		//Serial.println("timeToInhale 1:2 " + String(inhaleTime));
		return inhaleTime * 1000; 
	}


	if (conf.inhaleExhaleRatio == 3) {
		inhaleTime = timeToBreathe / 4.0f;
		//Serial.println("timeToInhale 1:2 " + String(inhaleTime));
		return inhaleTime * 1000;
	}


}


float getExhaleTime(int BPM) {

	float timeToBreathe = 60.0 / BPM;
	//Serial.println("timeToBreathe: " + String(timeToBreathe));
	float exhaleTime;

	if (conf.inhaleExhaleRatio == 1) {
		exhaleTime = timeToBreathe / 2.0f;
		//Serial.println("timeToInhale 1:1 " + String(inhaleTime));
		return exhaleTime * 1000;
	}

	if (conf.inhaleExhaleRatio == 2) {
		exhaleTime = (timeToBreathe / 3.0f) * 2.0f;
		//Serial.println("timeToInhale 1:2 " + String(inhaleTime));
		return exhaleTime * 1000;
	}


	if (conf.inhaleExhaleRatio == 3) {
		exhaleTime = (timeToBreathe / 4.0f) * 3.0f;
		//Serial.println("timeToInhale 1:2 " + String(inhaleTime));
		return exhaleTime * 1000;
	}

}


void debugAtInterval(String message) {

	unsigned long current_millis = millis();

	if (current_millis - lastDebug >= 1000) {

		Serial.println(message);
		lastDebug = millis();

	}


}


void setMotorSpeed(float timeToReach) {

	long stepsToGo = conf.motionLength - homePosition;
	//Serial.print("Steps to go: ");
	//Serial.println(stepsToGo);
	int stepsMultiplier = 1000;


	if (timeToReach < 1000) {
		stepsMultiplier = 2000;
	}

	if (timeToReach < 2000) {
		stepsMultiplier = 1100;
	}

	int speed = (stepsToGo * stepsMultiplier) / timeToReach;
	conf.stepperSpeed = speed;
	//Serial.print("Setting Motor Speed");
	//Serial.println(speed);
}


void setAirVolume() {

	int airVolPercentage = conf.airVol / (maxAirVol / 100);
	conf.motionLength = (maxMotionLength / 100) * airVolPercentage;
	

}

void serialCom() {
	
	int conf_changed = 0;

	if (HWSERIAL.available()) {
		input = HWSERIAL.readStringUntil('\n');
		StaticJsonDocument<200> command;

		DeserializationError error = deserializeJson(command, input.c_str());

		if (error) {
			HWSERIAL.print(F("deserializeJson() failed: "));
			HWSERIAL.println(error.c_str());
			return;
		}

		if (command["breathsPerMinute"]) {

			setBreathsPerMinuteParam(command["breathsPerMinute"]);
			conf_changed = 1;

		}

		if (command["ieRate"]) {

			setInhaleExhaleRateParam(command["ieRate"]);
			conf_changed = 1;
		}

		if (command["airVol"]) {

			setAirVolParam(command["airVol"]);
			conf_changed = 1;
		}

		//if (command["motionLength"]) {
		//
		//	setMotionLength(command["motionLength"]);
		//	conf_changed = 1;
		//	zeroed = false;
		//}



		//if (command["stepSpeed"]) {

		//	conf.stepperSpeed = command["stepSpeed"];


		//}

		//if (command["motor"]) {
		//	if (command["motor"] == "on") {
		//		motorShouldRun = true;
		//	}
		//	
		//	if (command["motor"] == "off") {

		//		motorShouldRun = false;

		//	}


		//}

	}

	if (conf_changed) {

		EEPROM_writeAnything(0, conf);
		conf_changed = 0;
		HWSERIAL.println("Saved Config");
	}


}


void pressureCalculator(float inputPressure) {

	float pressure = (inputPressure - 270) * 450;
	HWSERIAL.print("Pressure is: ");
	HWSERIAL.println(String(pressure / 10000));


}


void pressureRead() {
	
	if (readPressure) {

		digitalWrite(LED, HIGH);
		readVal1 = adc->adc0->analogRead(readPin); // read a new value, will return ADC_ERROR_VALUE if the comparison is false.

		//Serial.print("readVal1 read is ");
		//Serial.println(readVal1);

		readVal2 = adc->adc0->analogRead(readPin2); // read a new value, will return ADC_ERROR_VALUE if the comparison is false.

		//Serial.print("readVal1 read is ");
		//Serial.println(readVal1);

		digitalWrite(LED, LOW);
	}

}
