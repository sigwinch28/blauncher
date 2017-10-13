#pragma once

enum State { SAFE = 0, ARMED = 1, FUELLING = 2, FUELLED = 3, FIRING = 4 };


class Launcher {
private:
	State state;
	int pressure;
	int targetPressure;
public:
	Launcher();

	State getState();

	bool setState(State newState);
	bool allowed(State newState);

	bool arm();
	bool disarm();

	bool fuel();
	bool dumpFuel();

	bool setPressure(int pressure);
	int getPressure();

	bool setTargetPressure(int target);
	int getTargetPressure();

	bool fire();
	bool abort();
};