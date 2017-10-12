#include "Launcher.h"

Launcher::Launcher() : state(SAFE), pressure(0), targetPressure(0) { }

State Launcher::getState() {
    return state;
}

bool Launcher::arm() {
    if(state == SAFE) {
        state = ARMED;
        return true;
    }
    return false;
}

bool Launcher::disarm() {
    if (state >= ARMED) {
        abort();
        dumpFuel();
    state = SAFE;
    return true;
}
return false;
}

bool Launcher::fuel() {
    if (state == ARMED) {
        state = FUELLING;
        return true;
    }
    return false;
}

void Launcher::setPressure(int pressure) {
    this->pressure = pressure;
    if (state == FUELLING && pressure >= targetPressure) {
        state = FUELLED;
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
    if (state >= FUELLED) {
        abort();
        state = ARMED;
        return true;
    }
    return false;
}

bool Launcher::fire() {
    if (state == FUELLED) {
        state = FIRING;
        return true;
    }
    return false;
}


bool Launcher::abort() {
    if (state == FIRING) {
        state = FUELLED;
        return true;
    }
    return false;
}