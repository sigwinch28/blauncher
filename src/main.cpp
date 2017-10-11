#include <Arduino.h>

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

class Switch {
private:

  volatile bool changed;
  volatile bool pressed;
  unsigned long last_changed;
  unsigned long interval;
public:
  Switch(unsigned long n) : changed(false), pressed(false), interval(n) { };

  void set(bool v) {
    unsigned long t = millis();
    if (t >= last_changed + interval && v != pressed) {
      last_changed = t;
      pressed = v;
      changed = true;
    }
  }

  bool get() {
    bool v = pressed;
    changed = false;
    return v;
  }

  bool has_changed() {
    return changed;
  }
};

// TODO: put enum values in allcaps.
enum State { safe, armed, fuelling, fuelled, firing };

class Launcher {
private:
  State state;
  int pressure;
  int target_pressure;
public:
  Launcher() : state(safe), pressure(0), target_pressure(0) { }

  State get_state() {
    return state;
  }

  bool arm() {
    if(state == safe) {
      state = armed;
      return true;
    }
    return false;
  }

  bool disarm() {
    if (state >= armed) {
      abort();
      dump_fuel();
      state = safe;
      return true;
    }
    return false;
  }

  bool fuel() {
    if (state == armed) {
      state = fuelling;
      return true;
    }
    return false;
  }

  void set_pressure(int val) {
    pressure = val;
    if (state == fuelling && pressure >= target_pressure) {
      state = fuelled;
    }
  }

  int get_pressure() {
    return pressure;
  }

  bool set_target_pressure(int val) {
    if (state == safe) {
      target_pressure = val;
      return true;
    }
    return false;
  }

  int get_target_pressure() {
    return target_pressure;
  }

  bool dump_fuel() {
    if (state >= fuelled) {
      abort();
      state = armed;
      return true;
    }
    return false;
  }

  bool fire() {
    if (state == fuelled) {
      state = firing;
      return true;
    }
    return false;
  }

  bool abort() {
    if (state == firing) {
      state = fuelled;
      return true;
    }
    return false;
  }
};

Switch arm(50);
Switch fuel(50);
Switch fire(50);
Launcher launcher;
int pressure;

void do_switch_isr(Switch &sw, int pin) {
  sw.set(digitalRead(pin) == HIGH);
}

void arm_isr() {
  do_switch_isr(arm, ARM_PIN);
}

void fuel_isr() {
  do_switch_isr(fuel, FUEL_PIN);
}

void fire_isr() {
  do_switch_isr(fire, FIRE_PIN);
}

void setup() {
#ifdef DEBUG
  Serial.begin(BAUD_RATE);
  delay(1000);
  logln();
  logln("Rocket launcher v0");
#endif

  pinMode(ARM_PIN, INPUT);
  attachInterrupt(ARM_PIN, arm_isr, CHANGE);

  pinMode(FUEL_PIN, INPUT);
  attachInterrupt(FUEL_PIN, fuel_isr, CHANGE);

  pinMode(FIRE_PIN, INPUT);
  attachInterrupt(FIRE_PIN, fire_isr, CHANGE);
}
  
void do_pressure() {
  if(launcher.get_state() != safe) {
    return;
  }

  int val = constrain(analogRead(PRESSURE_PIN), PRESSURE_INPUT_MIN, PRESSURE_INPUT_MAX);
  int pressure = map(val, PRESSURE_INPUT_MIN, PRESSURE_INPUT_MAX, PRESSURE_OUTPUT_MIN, PRESSURE_OUTPUT_MAX);
  launcher.set_target_pressure(pressure);
}

void do_launcher_state() {
  log("Launcher - ");
  log("Target: "); log(launcher.get_target_pressure()); log("kPa");
  log(" | Pressure: "); log(launcher.get_pressure()); log("kPa");
  log(" | State: ");
  
  switch (launcher.get_state()) {
    case safe: log("SAFE"); break;
    case armed: log("ARMED"); break;
    case fuelling: log("FUELLING"); break;
    case fuelled: log("FUELLED"); break;
    case firing: log("FIRING"); break;
  }

  logln();
}

void do_fake_fuel() {
  if (launcher.get_state() == fuelling && launcher.get_pressure() < launcher.get_target_pressure()) {
    launcher.set_pressure(launcher.get_pressure() + 16);
  }
  if (launcher.get_state() < fuelling && launcher.get_pressure() > 0) {
    launcher.set_pressure(launcher.get_pressure() - 64);
  }
}

void loop() {
  unsigned long t = millis();

  do_launcher_state();
  do_pressure();

  if (arm.has_changed()) {
    if (arm.get()) {
      launcher.arm();
    } else {
      launcher.disarm();
    }
  }

  if (fuel.has_changed()) {
    if (fuel.get()) {
      launcher.fuel();
    } else {
      launcher.dump_fuel();
    }
  }

  if (fire.has_changed()) {
    if (fire.get()) {
      launcher.fire();
    } else {
      launcher.abort();
    }
  }
  
  do_fake_fuel();
  delay(200 - (millis() - t));
}
