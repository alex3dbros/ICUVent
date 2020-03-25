long hardLimiter(long min, long max, long value) {

	if (value > max) {
		return max;
	}

	if (value < min) {
		return min;
	}

	return value;
}

void zero_position() {
	pinMode(ZERO_POS, INPUT_PULLUP);
	while (!digitalRead(ZERO_POS))
	{
		stepper.step(-1);
	}

	while (digitalRead(ZERO_POS))
	{
		stepper.step(1);
	}

	ventPos = 0;
	zeroed = 1;
	menu_select();
}




void zeroSwitchCheck() {

	unsigned long pressed_time;

	unsigned long current_millis = millis();

	while (digitalRead(ZERO_POS))
	{
		unsigned long time_now = millis();
		stepper.step(1);
		if (time_now - current_millis > 100) {
			ventPos = 0;
			break;

		}

	}

}


void ventWatchdog(int targetPosition) {

	while (ventPos != targetPosition) {
		displayControl();


		if ((targetPosition - ventPos) > 0) {
			stepper.step(1);
			ventPos += 1;
			zeroSwitchCheck();
			continue;

		}

		if ((targetPosition - ventPos) < 0) {
			stepper.step(-1);
			ventPos -= 1;
			zeroSwitchCheck();
		}
	}

}