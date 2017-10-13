#include <Arduino.h>

#include "Switch.h"

// Set the provided pin to input, and read using the default debounce value.
Switch::Switch(int pin) : Switch(pin, 200) { }

// Set the provided pin to input, and read using the provided debounce value.
Switch::Switch(int pin, unsigned long debounce) :
pin(pin), changed(false), value(LOW), lastChanged(0), debounce(debounce)
{
	pinMode(pin, INPUT);
}

// Read the switch pin.
// Updates the switch state iff the debounce interval has passed and the value
// is different. Also sets the changed flag.
bool Switch::read() {
	unsigned long now = millis();
	int newValue = digitalRead(pin);
	if (now > lastChanged + debounce && newValue != value) {
		value = newValue;
		changed = true;
		return true;
	}
	return false;
}

// Returns the stored value and unsets the changed flag.
bool Switch::get() {
	changed = false;
	return value;
}

// Returns true if the changed flag is set. Does not unset it.
bool Switch::hasChanged() {
	return changed;
}