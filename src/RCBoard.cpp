#include "RCBoard.h"

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

RCBoard::RCBoard() {
  _began = false;

  for (int i = 0; i < 2; i++) {
    _ledState[i]   = false;
    _lastRaw[i]    = false;
    _stable[i]     = false;
    _lastChange[i] = 0;
  }

  _kp = 0.0;
  _ki = 0.0;
  _kd = 0.0;
  _lastError = 0;
  _integral  = 0;

  _warnedNoBegin = false;
  _warnedButton  = false;
  _warnedLed     = false;
}

void RCBoard::begin() {
  pinMode(STDBY, OUTPUT);
  pinMode(AIN1,  OUTPUT);
  pinMode(AIN2,  OUTPUT);
  pinMode(BIN1,  OUTPUT);
  pinMode(BIN2,  OUTPUT);
  pinMode(PWMA,  OUTPUT);
  pinMode(PWMB,  OUTPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  // INPUT_PULLUP holds the pin HIGH on its own, and pressing the button
  // pulls it down to GND. That is why a pressed button reads LOW, and why
  // isPressed() flips it round for you.
  pinMode(LB, INPUT_PULLUP);
  pinMode(RB, INPUT_PULLUP);

  // The TB6612FNG has a master on/off pin. While it is LOW both motors are
  // dead no matter what else you send. Forgetting this is the most common
  // reason a robot sits there doing nothing, so the library just does it.
  digitalWrite(STDBY, HIGH);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  _ledState[0] = false;
  _ledState[1] = false;

  _began = true;
}

// ---------------------------------------------------------------------------
// Motors
// ---------------------------------------------------------------------------

void RCBoard::drive(int speedA, int speedB) {
  if (!_checkBegan()) return;

  // Anything past full speed is simply full speed. Quietly trimmed so a
  // stray big number never reaches analogWrite().
  speedA = constrain(speedA, -255, 255);
  speedB = constrain(speedB, -255, 255);

  _driveMotor(AIN1, AIN2, PWMA, speedA);
  _driveMotor(BIN1, BIN2, PWMB, speedB);
}

void RCBoard::stop() {
  drive(0, 0);
}

// Sends one motor forwards, backwards, or into a brake.
//
// The driver chip decides direction from the two direction pins:
//   one HIGH, one LOW  -> spin, whichever way round they are
//   both HIGH          -> brake, the motor actively resists turning
//   both LOW           -> coast, the motor free-wheels to a halt
//
// A speed of 0 brakes rather than coasts, because a robot that stops where
// you told it to is far easier to drive than one that drifts on.
void RCBoard::_driveMotor(int in1, int in2, int pwm, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(pwm, speed);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(pwm, -speed);   // -speed is how fast, without the minus
  } else {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, HIGH);
    analogWrite(pwm, 0);
  }
}

// ---------------------------------------------------------------------------
// PID steering
// ---------------------------------------------------------------------------

void RCBoard::setPID(float kp, float ki, float kd) {
  _kp = kp;
  _ki = ki;
  _kd = kd;
}

// Turns "how far off the line am I" into "how hard should I steer".
//
// Feed it the error from your sensor array, for instance
// sensors.getError() from an ARC8 or ARC16. Three parts add up:
//
//   P  how far off you are right now
//   I  how far off you have been, all added together
//   D  how fast that is changing, which damps the wobble
//
// Then steer with the answer:
//   Robot.drive(base - correction, base + correction);
int RCBoard::getCorrection(int error) {
  if (!_checkBegan()) return 0;

  int P = error;

  // _integral keeps every error we have ever had, added together. One error
  // can be thousands, so an int would fill up and wrap round to a huge
  // negative number within a few loops, yanking the robot the wrong way.
  // A long has room for hours of driving.
  _integral += error;

  int D = error - _lastError;
  _lastError = error;

  float output = _kp * P + _ki * _integral + _kd * D;
  return (int)output;
}

// ---------------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------------

bool RCBoard::_readButton(int button) {
  // Pressed pulls the pin down to GND, so LOW means pressed.
  return digitalRead(button) == LOW;
}

bool RCBoard::isPressed(int button, unsigned int debounceMs) {
  if (!_checkBegan()) return false;

  int i = _buttonIndex(button);
  if (i < 0) return false;

  bool reading = _readButton(button);

  // No debounce asked for: hand back exactly what the pin says, right now.
  if (debounceMs == 0) return reading;

  // Debounce asked for. A real push button does not switch cleanly - the
  // metal contacts bounce apart and back together for a few milliseconds,
  // so one press can look like several. The fix is to ignore a new reading
  // until it has stayed the same for long enough to be believable.
  //
  // Every time the reading flips we restart the clock. Only once it has
  // held steady for debounceMs do we accept it as the real answer. That
  // means this needs calling regularly - inside loop() is ideal.
  if (reading != _lastRaw[i]) {
    _lastRaw[i]    = reading;
    _lastChange[i] = millis();
  }

  if (millis() - _lastChange[i] >= debounceMs) {
    _stable[i] = reading;
  }

  return _stable[i];
}

void RCBoard::waitFor(int button) {
  if (!_checkBegan()) return;

  // Check the button is real first. Without this a typo would hang the
  // sketch forever on a pin that is never going to go low.
  if (_buttonIndex(button) < 0) return;

  while (!_readButton(button)) {
    // Wait here doing nothing until someone presses it.
  }
}

int RCBoard::_buttonIndex(int button) {
  if (button == LB) return 0;
  if (button == RB) return 1;

  if (!_warnedButton) {
    Serial.println(F("RCBoard: unknown button"));
    _warnedButton = true;
  }
  return -1;
}

// ---------------------------------------------------------------------------
// LEDs
// ---------------------------------------------------------------------------

void RCBoard::ledOn(int led) {
  if (!_checkBegan()) return;

  int i = _ledIndex(led);
  if (i < 0) return;

  _ledState[i] = true;
  digitalWrite(led, HIGH);
}

void RCBoard::ledOff(int led) {
  if (!_checkBegan()) return;

  int i = _ledIndex(led);
  if (i < 0) return;

  _ledState[i] = false;
  digitalWrite(led, LOW);
}

void RCBoard::ledToggle(int led) {
  if (!_checkBegan()) return;

  int i = _ledIndex(led);
  if (i < 0) return;

  // We remember what we last told the LED to do, rather than asking the pin,
  // so this works the same way on every board.
  _ledState[i] = !_ledState[i];
  digitalWrite(led, _ledState[i] ? HIGH : LOW);
}

int RCBoard::_ledIndex(int led) {
  if (led == LED1) return 0;
  if (led == LED2) return 1;

  if (!_warnedLed) {
    Serial.println(F("RCBoard: unknown LED"));
    _warnedLed = true;
  }
  return -1;
}

// ---------------------------------------------------------------------------
// Private checks
// ---------------------------------------------------------------------------

bool RCBoard::_checkBegan() {
  if (_began) return true;

  if (!_warnedNoBegin) {
    Serial.println(F("RCBoard: call begin() first"));
    _warnedNoBegin = true;
  }
  return false;
}
