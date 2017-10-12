#pragma once

enum State { SAFE, ARMED, FUELLING, FUELLED, FIRING };

class Launcher {
private:
  State state;
  int pressure;
  int targetPressure;
public:
  Launcher();

  State getState();

  bool arm();
  bool disarm();

  bool fuel();
  bool dumpFuel();

  void setPressure(int pressure);
  int getPressure();

  bool setTargetPressure(int target);
  int getTargetPressure();

  bool fire();
  bool abort();
};