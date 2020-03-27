/*
 Name:		ArduinoSlave.ino
 Created:	3/25/2020 9:00:42 PM
 Author:	Alex Gradea
*/
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
//#include <Stepper.h>
#include <Wire.h> 
//#include <ArduinoJson.h>
#include <AccelStepper.h>

#define STEPPER1_DIR_PIN 22
#define STEPPER1_STEP_PIN 21

AccelStepper ventMotor(AccelStepper::DRIVER, STEPPER1_STEP_PIN, STEPPER1_DIR_PIN);

int millisbetweenSteps = 1;

#define ENdrv 23
#define ZERO_POS 20 // Endstop

long counter = 0;
int State;
int LastState;

int menu_page = 0;

int maxBreathsPM = 40;
int minBreathsPM = 3;

int maxAirVol = 600;
int minAirVol = 200;

int maxTimeBB = 100;
int minTimeBB = 0;

int maxInhaleRatio = 3;
int minInhaleRatio = 1;

long minMotionLength = 10;
long maxMotionLength = 10200; // hard limiting the movement of the mechanics

int homePosition = 100;
int zeroed = 0;
int ventPos = 0;

int minStepperSpeed = 100;
int maxStepperSpeed = 20000;
int acceleration = 20000;

unsigned long last_watchdog_update = 0;
unsigned long last_display_update = 0;

bool needInhale = true;
bool needExhale = false;

bool inhaling = false;
bool exhaling = false;
bool motorShouldRun = false;

String input;

unsigned long lastDebug = 0;



unsigned long debugInhaleTime = 0;


struct config
{
	int breathsPerMinute;
	int airVol;
	int inhaleExhaleRatio;
	long motionLength;
	int stepperSpeed;
};

typedef struct config Config;
Config conf;

void setup() {

	ventMotor.setMaxSpeed(1000);
	ventMotor.setAcceleration(1000);

	pinMode(ENdrv, OUTPUT);
	digitalWrite(ENdrv, HIGH); // Stepper driver ENABLE ! 

	//Define the pins as inputs
	pinMode(ZERO_POS, INPUT_PULLUP);

	Serial.begin(9600); //Start serial com so we could print the value on the serial monitor
	// Reads the initial state of the clock pin

	loadOrwriteDefaults();

}

void loop() {

	if (!zeroed) {
		delay(100);
		zero_position();
		motorShouldRun = true;
		needInhale = true;
	}

	serialCom();
	
	unsigned long current_millis = millis();
	unsigned long lastPrint;

	
	float timeToInhale = getInhaleTime(conf.breathsPerMinute);
	float timeToExhale = getExhaleTime(conf.breathsPerMinute);


	if (needInhale || inhaling) {

		if (needInhale) {


			debugInhaleTime = millis();

			setMotorSpeed(timeToInhale);
			setAirVolume();
			ventWatchdog(conf.motionLength, "Inhale");
			needInhale = false;
		}
		
		if (inhaling) {
			if (ventMotor.currentPosition() == conf.motionLength) {

				needExhale = true;
				inhaling = false;
				Serial.print("Time to inhale: ");
				Serial.println(current_millis - debugInhaleTime);

				Serial.print("At stepper Speed: ");
				Serial.println(conf.stepperSpeed);

			}

		}

	}


	if (needExhale || exhaling) {

		if (needExhale) {
			setMotorSpeed(timeToExhale);
			ventWatchdog(homePosition, "Exhale");
			needExhale = false;
		}

		if (exhaling) {

			if (ventMotor.currentPosition() == homePosition) {

				needInhale = true;
				exhaling = false;

			}

		}

	}

	if (motorShouldRun) {
		ventMotor.run();
		homeSwitchCheck();
	}

	if (!motorShouldRun) {
		ventMotor.stop();
	}
	


}
