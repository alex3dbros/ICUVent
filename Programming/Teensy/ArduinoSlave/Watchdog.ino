void ventWatchdog(int targetPosition, String action) {

	if (action == "Inhale") {

		motor.setMaxSpeed(conf.stepperSpeed);
		motor.setTargetAbs(targetPosition);
		controller.moveAsync(motor);
		//Serial.println("Started motor movement");
		inhaling = true;

	}

	if (action == "Exhale") {

		motor.setMaxSpeed(conf.stepperSpeed);
		motor.setTargetAbs(targetPosition);
		controller.moveAsync(motor);
		//Serial.println("Started motor movement exhaling");
		exhaling = true;
	}

}


void sensorsWatchdog() {

	unsigned long current_millis = millis();
	

	if (current_millis - last_sensorWatchdog_update >= 10) {
		pressureRead();

		//String message = "{\"pressure\":" + String(readVal1) + ",\"airflow\":" + String(readVal2) + ",\"tidalvol\":" + String(conf.airVol) +
		//	",\"resprate\":" + String(conf.breathsPerMinute) + ",\"ieratio\":" + String(conf.inhaleExhaleRatio) + "}";

		String message = "{\"pressure\":" + String(readVal1) + ",\"airflow\":" + String(readVal2) +  "}";

		HWSERIAL.println(message);

		//byte message[] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };

		//HWSERIAL.write(message, sizeof(message));


		last_sensorWatchdog_update = millis();
	}

}



