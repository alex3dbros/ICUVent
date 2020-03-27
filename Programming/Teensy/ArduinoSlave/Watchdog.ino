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
