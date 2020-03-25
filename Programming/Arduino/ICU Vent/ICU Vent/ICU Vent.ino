/*
 Name:		ICU_Vent.ino
 Created:	3/21/2020 2:37:30 PM
 Author:	Alex Gradea (3D-Brothers.SRL)
*/

#include <EEPROM.h>
#include "EEPROMAnything.h"

#include "Button2.h";

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <Stepper.h>

#define STEPS 12800 // Number of steps per revolution
Stepper stepper(STEPS, 6, 7);
int millisbetweenSteps = 1;

#define ENdrv 8
#define clk 3
#define data 4

#define ZERO_POS 2 // Endstop
#define step_pin 6 // Pin 3 connected to Steps pin 
#define dir_pin 7  // Pin 2 connected to Direction pin

#define BUTTON_A_PIN  5
Button2 buttonA = Button2(BUTTON_A_PIN);

long counter = 0;
int State;
int LastState;

int menu_page = 0;

int maxBreathsPM = 40;
int minBreathsPM = 3;

int maxAirVol = 100;
int minAirVol = 10;

int maxTimeBB = 100;
int minTimeBB = 0;

long minMotionLength = 10;
long maxMotionLength = 2000;
int zeroed = 0;
int ventPos = 0;
int breathingSpdMult = 1500;

unsigned long last_watchdog_update = 0;
unsigned long last_display_update = 0;

String firstLine = "DIY Ventilator v1.0";
String secondLine = "Starting";

String firstLcdText = "";
String secondLcdText = "";
int needInhale = 1;
int needExhale = 0;

struct config
{
	int breathingSpeed;
	int airVol;
	int timeBetweenBreaths;
	int inhaleToExhale;
	long motionLength;
};

typedef struct config Config;
Config conf;


void loadOrwriteDefaults() {

	int canWriteDefaults;
	EEPROM_readAnything(80, canWriteDefaults);
	//canWriteDefaults = -1;

	if (canWriteDefaults == -1) {
		
		conf.breathingSpeed = 12;
		conf.airVol = 80;
		conf.timeBetweenBreaths = 0;
		conf.inhaleToExhale = 0;
		conf.motionLength = 5000;
		EEPROM_writeAnything(0, conf);
		EEPROM_writeAnything(80, 1);
	}

	if (canWriteDefaults == 1) {

		EEPROM_readAnything(0, conf);

	}
}


void setup() {

	pinMode(ENdrv, OUTPUT);
	digitalWrite(ENdrv, HIGH); // Stepper driver ENABLE ! 

    //Define the pins as inputs
	pinMode(clk, INPUT_PULLUP);
	pinMode(data, INPUT_PULLUP);
	pinMode(BUTTON_A_PIN, INPUT_PULLUP);
	pinMode(ZERO_POS, INPUT_PULLUP);

	buttonA.setClickHandler(ButtonHandler);
	buttonA.setLongClickHandler(ButtonHandler);

	Serial.begin(9600); //Start serial com so we could print the value on the serial monitor

	// Reads the initial state of the clock pin
	LastState = digitalRead(clk);

	lcd.init();                      // initialize the lcd 

	// Print a message to the LCD.
	lcd.backlight();
	lcd.setCursor(1, 0);
	lcd.print("DIY Ventilator v1.0");
	lcd.setCursor(5, 1);
	lcd.print("Starting");

	loadOrwriteDefaults();
	stepper.setSpeed(conf.breathingSpeed * breathingSpdMult);

}

void loop() {

	if (!zeroed) {
		delay(1000);
		zero_position();

	}
	displayControl();


	unsigned long current_millis = millis();

	if (current_millis - last_watchdog_update >= float(conf.timeBetweenBreaths) * 100 && needInhale)
	{
		
		ventWatchdog(conf.motionLength * (conf.airVol / 100.0));

		last_watchdog_update = millis();
		needInhale = 0;
		needExhale = 1;

	}

	current_millis = millis();

	if (current_millis - last_watchdog_update >= float(conf.inhaleToExhale) * 100 && needExhale) {

		ventWatchdog(20);

		needInhale = 1;
		needExhale = 0;
		
		last_watchdog_update = millis();

	}

}



