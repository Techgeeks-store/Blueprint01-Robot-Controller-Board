/*
  StartupSequence - RCBoard

  The pattern almost every competition robot uses: don't do anything until
  a human says go.

  You place the robot on the track, press the left button to run whatever
  setup step you need, then press the right button when you are ready for it
  to actually drive off. Nothing happens by surprise.

    press LB  ->  LED1 on, do your setup step, LED1 off
    press RB  ->  LED2 on, robot drives

  waitFor() is what makes this work. It parks your sketch on that line until
  the button is pressed, and then carries on.

  This is where you would slot in a sensor calibration sweep - the ARC8 or
  ARC16 libraries both do that with a read-and-calibrate loop. Here the
  setup step is just a blink, so the sketch stands on its own.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <RCBoard.h>

RCBoard Robot;

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Serial.println("Press the LEFT button to start setup.");
  Robot.waitFor(LB);              // sketch pauses right here

  Robot.ledOn(LED1);
  Serial.println("Setting up...");

  // Your real setup step goes here - a sensor calibration sweep, for
  // instance. Blinking for two seconds stands in for it.
  for (int i = 0; i < 10; i++) {
    Robot.ledToggle(LED1);
    delay(200);
  }

  Robot.ledOff(LED1);
  Serial.println("Setup done.");

  Serial.println("Press the RIGHT button to drive.");
  Robot.waitFor(RB);              // and pauses again here

  Robot.ledOn(LED2);
  Serial.println("Going!");
}

void loop() {
  // Drive forward. Press the left button at any time to stop.
  Robot.drive(120, 120);

  if (Robot.isPressed(LB, 20)) {
    Robot.stop();
    Robot.ledOff(LED2);
    Serial.println("Stopped. Press RIGHT to go again.");
    Robot.waitFor(RB);
    Robot.ledOn(LED2);
  }
}
