/*
  LineFollowerPID - RCBoard

  A complete line follower, using an ARC8 sensor array on the sensor rail.

  The two libraries split the job between them:

    the ARC8 library  reads the track and tells you how far off you are
    this library      turns that into steering, and drives the motors

  So the sensor array has no PID of its own. getError() gives you the
  position, and Robot.getCorrection() does the maths.

  A NEGATIVE correction means the line went right, because sensor 0 is at
  the right-hand end of the array and carries a negative weight. That is why
  the correction is subtracted from the left wheel and added to the right.

  Tuning, one number at a time:
    1. Start with kp only, ki and kd at 0.
    2. Raise kp until it follows the line but wobbles.
    3. Back kp off a little, then add kd to calm the wobble.
    4. Leave ki at 0 unless it always rides to one side.

  Needs the ARC8 library installed as well as this one.
*/

#include <RCBoard.h>
#include <ARC8.h>

RCBoard Robot;
ARC8 sensors;

int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int values[8];

const int BASE_SPEED = 120;

void setup() {
  Serial.begin(9600);
  Robot.begin();
  Robot.setPID(0.03, 0.0, 0.8);   // kp, ki, kd

  sensors.begin(pins);
  sensors.setLine(BLACK);

  // Press the left button, then sweep the robot across the line while it
  // calibrates.
  Serial.println(F("Press the left button to calibrate."));
  Robot.waitFor(LB);
  Robot.ledOn(LED1);

  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }
  Robot.ledOff(LED1);

  Serial.println(F("Press the right button to run."));
  Robot.waitFor(RB);
  Robot.ledOn(LED2);
}

void loop() {
  // No delay here. A line follower needs to notice it is drifting at once.
  sensors.read(values);

  // If every sensor loses the line, getError() keeps giving the last
  // position it was sure about, so the robot carries on turning the way it
  // already was and usually finds the line again.
  int correction = Robot.getCorrection(sensors.getError());

  Robot.drive(BASE_SPEED - correction, BASE_SPEED + correction);
}
