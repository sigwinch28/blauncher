#include <Arduino.h>

#include "Launcher.h"
#include "Switch.h"

#define BAUD_RATE 921600

#define ARM_PIN D15
#define FUEL_PIN D14
#define FIRE_PIN D12
#define PRESSURE_PIN A0
#define PIEZO_PIN D8

#define TONE_DURATION 300

#define ARM_TONE 2550
#define FUEL_TONE 3038
#define FIRE_TONE 3830

#define PRESSURE_INPUT_MIN 0
#define PRESSURE_INPUT_MAX 980

#define PRESSURE_OUTPUT_MIN 0
#define PRESSURE_OUTPUT_MAX 1024



#define DEBUG

#ifdef DEBUG
#define logln(str) Serial.println(str)
#define log(str) Serial.print(str)
#else
#define logln(str) do {} while(0)
#define log(str) do {} while(0)
#endif

Switch arm(50);
Switch fuel(50);
Switch fire(50);
Launcher launcher;
bool updateRequired;

void arm_isr() {
	arm.read();
}

void fuel_isr() {
	fuel.read();
}

void fire_isr() {
	fire.read();
}

void setup() {
	#ifdef DEBUG
	Serial.begin(BAUD_RATE);
	delay(1000);
	logln();
	logln("Rocket launcher v0");
	#endif
	
	attachInterrupt(ARM_PIN, arm_isr, CHANGE);
	attachInterrupt(FUEL_PIN, fuel_isr, CHANGE);
	attachInterrupt(FIRE_PIN, fire_isr, CHANGE);
}

// Update the launcher's target pressure using an analog input mapped to between
// 0 and 1024kPa.
//
// Doesn't do anything unless the launcher is in the SAFE state, as this is the
// only time the state can be updated.
//
// Returns true if the pressure was updated.
bool updateTargetPressure() {
	if(launcher.getState() != SAFE) {
		return false;
	}
	
	int val = constrain(analogRead(PRESSURE_PIN), PRESSURE_INPUT_MIN, PRESSURE_INPUT_MAX);
	int pressure = map(val, PRESSURE_INPUT_MIN, PRESSURE_INPUT_MAX, PRESSURE_OUTPUT_MIN, PRESSURE_OUTPUT_MAX);
	if (pressure != launcher.getTargetPressure()) {
		launcher.setTargetPressure(pressure);
		return true;
	}
	return false;
}

// Read the switches and update the launcher state if necessary.
//
// Returns true if the state was changed.
bool updateLauncherState() {
	bool changed = false;
	// Arm switch - on: arm, off: disarm.
	if (arm.hasChanged()) {
		if (arm.get() == HIGH) {
			changed = launcher.arm();
		} else {
			changed = launcher.disarm();
		}
	}
	
	// Fuel switch - on: fuel, off: dump.
	if (fuel.hasChanged()) {
		if (fuel.get() == HIGH) {
			changed = launcher.fuel();
		} else {
			changed = launcher.dumpFuel();
		}
	}
	
	// Fire switch - on: fire, off: abort.
	if (fire.hasChanged()) {
		if (fire.get()) {
			changed = launcher.fire();
		} else {
			changed = launcher.abort();
		}
	}
	return changed;
}

void printLauncherState() {
	log("Launcher - ");
	log("Target: "); log(launcher.getTargetPressure()); log("kPa");
	log(" | Pressure: "); log(launcher.getPressure()); log("kPa");
	log(" | State: ");
	
	switch (launcher.getState()) {
		case SAFE: log("SAFE"); break;
		case ARMED: log("ARMED"); break;
		case FUELLING: log("FUELLING"); break;
		case FUELLED: log("FUELLED"); break;
		case FIRING: log("FIRING"); break;
	}
	
	logln();
}

// We don't really have a compressor or pressure transducer, so
// we artificially raise and lower the pressure when the rocket is
// in the right state.
bool doFakeFuel() {
	if (launcher.getState() == FUELLING && launcher.getPressure() < launcher.getTargetPressure()) {
		launcher.setPressure(launcher.getPressure() + 16);
		return true;
	}
	if (launcher.getState() < FUELLING && launcher.getPressure() > 0) {
		launcher.setPressure(launcher.getPressure() - 64);
		return true;
	}
	return false;
}

void loop() {
	unsigned long t = millis();
	
	bool stateUpdated = updateLauncherState();
	bool targetPressureUpdated = updateTargetPressure();
	bool pressureUpdated = doFakeFuel();
	
	if (stateUpdated || targetPressureUpdated || pressureUpdated) {
		printLauncherState();
	}
	
	delay(200 - (millis() - t));
}
