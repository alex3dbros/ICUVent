void ButtonHandler(Button2& btn) {
	switch (btn.getClickType()) {
	case SINGLE_CLICK:
		menu_handeler();
		break;

	case LONG_CLICK:
		menu_select();
		save_handeler();
		break;
	}
}

void save_handeler() {

	EEPROM_writeAnything(0, conf);
	lcdUpdate("Saved To Mem", "");
	delay(1000);
	menu_page = 0;
	menu_select();

}


void menu_handeler() {

	if (menu_page >= 0) {
		menu_page = menu_page + 1;
	}

	if (menu_page > 6) {
		menu_page = 0;
	}



	if (menu_page == 1) {
		counter = conf.breathingSpeed;
	}

	if (menu_page == 2) {
		counter = conf.airVol;
	}

	if (menu_page == 3) {
		counter = conf.timeBetweenBreaths;
	}

	if (menu_page == 4) {
		counter = conf.inhaleToExhale;
	}

	if (menu_page == 5) {
		counter = conf.motionLength;
	}

	menu_select();
}

void menu_select() {

	if (menu_page == 0) {
		menu_0();
	}
	if (menu_page == 1) {
		menu_1();
	}
	if (menu_page == 2) {
		menu_2();
	}
	if (menu_page == 3) {
		menu_3();
	}

	if (menu_page == 4) {
		menu_4();
	}

	if (menu_page == 5) {
		menu_5();
	}
	if (menu_page == 6) {
		menu_6();
	}

}

void lcdUpdate(String fLine, String sLine) {

	if (fLine != firstLcdText || sLine != secondLcdText) {

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(fLine);

		lcd.setCursor(0, 1);
		lcd.print(sLine);

		firstLcdText = fLine;
		secondLcdText = sLine;

	}

}




void menu_0() {

	char firstBuffer[100];
	char secondBuffer[100];

	sprintf(firstBuffer, "SPD VOL RST ITE");
	secondLine = String(conf.breathingSpeed) + "  " + String(conf.airVol) + " " + String(float(conf.timeBetweenBreaths) / 10.0) + " " + String(float(conf.inhaleToExhale) / 10.0);
	firstLine = String(firstBuffer);
}
void menu_1() {
	char firstBuffer[100];
	char secondBuffer[100];

	conf.breathingSpeed = hardLimiter(minBreathsPM, maxBreathsPM, counter);
	counter = conf.breathingSpeed;

	sprintf(firstBuffer, "1) Breathing Speed");
	sprintf(secondBuffer, "  SPD: %i", conf.breathingSpeed);

	firstLine = String(firstBuffer);
	secondLine = String(secondBuffer);

	stepper.setSpeed(conf.breathingSpeed * 3);
}
void menu_2() {
	char firstBuffer[100];
	char secondBuffer[100];

	conf.airVol = hardLimiter(minAirVol, maxAirVol, counter);
	counter = conf.airVol;

	sprintf(firstBuffer, "2) Volume of air");
	sprintf(secondBuffer, "  VOL: %i%%", conf.airVol);

	firstLine = String(firstBuffer);
	secondLine = String(secondBuffer);

}
void menu_3() {
	char firstBuffer[100];
	char secondBuffer[100];

	conf.timeBetweenBreaths = hardLimiter(minTimeBB, maxTimeBB, counter);
	counter = conf.timeBetweenBreaths;

	sprintf(firstBuffer, "3) T Between Breaths");

	firstLine = String(firstBuffer);
	secondLine = "  RST: " + String(float(conf.timeBetweenBreaths) / 10.0) + " sec";
}

void menu_4() {
	char firstBuffer[100];
	char secondBuffer[100];

	conf.inhaleToExhale = hardLimiter(minTimeBB, maxTimeBB, counter);
	counter = conf.inhaleToExhale;

	sprintf(firstBuffer, "4) Inhale to Exhale");

	firstLine = String(firstBuffer);
	secondLine = "  ITE: " + String(float(conf.inhaleToExhale) / 10.0) + " sec";

}

void menu_5() {
	char firstBuffer[100];
	char secondBuffer[100];

	conf.motionLength = hardLimiter(minMotionLength, maxMotionLength * 30, counter);

	counter = conf.motionLength;

	sprintf(firstBuffer, "5) Motion Length");
	sprintf(secondBuffer, "  Steps: %i", conf.motionLength);

	firstLine = String(firstBuffer);
	secondLine = String(secondBuffer);

}



void menu_6() {

	char firstBuffer[100];
	char secondBuffer[100];


	sprintf(firstBuffer, "Hold 2s to Save");
	sprintf(secondBuffer, "");

	firstLine = String(firstBuffer);
	secondLine = String(secondBuffer);
	counter = 0;

}



void displayControl() {
	int incrementer = 1;

	buttonA.loop();
	State = digitalRead(clk);
	unsigned long current_millis = millis();

	if (menu_page == 5) {
		incrementer = 100;

	}

	if (State != LastState) {
		if (digitalRead(data) != State) {

			counter += incrementer;
		}
		else {
			counter -= incrementer;
		}

		menu_select();
	}

	if (current_millis - last_display_update >= 100) {
		lcdUpdate(firstLine, secondLine);
		last_display_update = current_millis;
	}


	LastState = State;

}