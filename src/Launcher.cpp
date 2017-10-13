#include "Launcher.h"

Launcher::Launcher() : state(SAFE), pressure(0), targetPressure(0) { }

State Launcher::getState() {
	return state;
}

bool Launcher::allowed(State newState) {
	if (newState == SAFE) {
		return true;
	}
	if (newState == FUELLED && state == FUELLING) {
		return pressure >= targetPressure;
	}
	return newState == state-1 || newState == state+1;
}

bool Launcher::setState(State newState) {
	if(allowed(newState)) {
		state = newState;
		return true;
	}
	return false;
}

bool Launcher::arm() {
	return setState(ARMED);
}

bool Launcher::disarm() {
	abort();
	dumpFuel();
	return setState(SAFE);
}

bool Launcher::fuel() {
	return setState(FUELLING);
}

bool Launcher::setPressure(int pressure) {
	this->pressure = pressure;
	if (state == FUELLING && pressure >= targetPressure) {
		return setState(FUELLED);
	} else if (state == FUELLED && pressure <= targetPressure) {
		return setState(FUELLING);
	}
}

int Launcher::getPressure() {
	return pressure;
}

bool Launcher::setTargetPressure(int target) {
	if (state == SAFE) {
		targetPressure = target;
		return true;
	}
	return false;
}

int Launcher::getTargetPressure() {
	return targetPressure;
}

bool Launcher::dumpFuel() {
	abort();
	return setState(ARMED);
}

bool Launcher::fire() {
	return setState(FIRING);
}


bool Launcher::abort() {
	return setState(FUELLED);
}