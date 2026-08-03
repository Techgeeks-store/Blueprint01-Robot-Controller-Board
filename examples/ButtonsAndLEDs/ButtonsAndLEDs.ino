/*
  ButtonsAndLEDs - RCBoard

  The left button turns both LEDs on. The right button turns them off.

  About the buttons: they are wired so that pressing one connects the pin to
  GND. That means a plain digitalRead() gives you 0 when pressed, which is
  backwards from what you would expect. isPressed() flips it round, so:

      if (Robot.isPressed(LB))   // true when you are holding it down

  Use LB for the left button and RB for the right one. Use LED1 and LED2 for
  the lights. Those names come straight from the board.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <RCBoard.h>

RCBoard Robot;

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Serial.println("Press the left button for lights on, right for off.");
}

void loop() {
  if (Robot.isPressed(LB)) {
    Robot.ledOn(LED1);
    Robot.ledOn(LED2);
    Serial.println("left button - lights on");
  }

  if (Robot.isPressed(RB)) {
    Robot.ledOff(LED1);
    Robot.ledOff(LED2);
    Serial.println("right button - lights off");
  }
}
