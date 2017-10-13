#pragma once

class Switch {
private:
	int pin;
	
	volatile bool changed;
	volatile int value;
	
	unsigned long lastChanged;
	unsigned long debounce;
public:
	Switch(int pin);
	Switch(int pin, unsigned long debounce);
	
	bool read();
	bool get();
	bool hasChanged();
};