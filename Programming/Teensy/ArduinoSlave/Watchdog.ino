void ventWatchdog(int targetPosition, String action) {

	if (action == "Inhale") {

		ventMotor.setSpeed(conf.stepperSpeed);
		ventMotor.setMaxSpeed(conf.stepperSpeed);

		ventMotor.moveTo(targetPosition);
		inhaling = true;

	}

	if (action == "Exhale") {
		ventMotor.setSpeed(conf.stepperSpeed);
		ventMotor.setMaxSpeed(conf.stepperSpeed);
		ventMotor.moveTo(30);
		exhaling = true;
	}

}


void sensorsWatchdog() {

	unsigned long current_millis = millis();
	

	if (current_millis - last_sensorWatchdog_update >= 10) {
		pressureRead();

		String message = "{\"pressure\":" + String(readVal1) + ",\"airflow\":" + String(readVal2) + ",\"tidalvol\":" + String(conf.airVol) +
			",\"resprate\":" + String(conf.breathsPerMinute) + ",\"ieratio\":" + String(conf.inhaleExhaleRatio) + "}";
		Serial.println(message);
		//Serial.print(readVal1);
		//Serial.print(",");
		//Serial.println(readVal2);

		last_sensorWatchdog_update = millis();
	}

}


