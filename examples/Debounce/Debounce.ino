/*
  Debounce - RCBoard

  Counting button presses, and why the extra number matters.

  A push button is two pieces of metal springing together. For a few
  thousandths of a second after you press it, they bounce apart and back
  again several times. The Arduino is fast enough to see every one of those
  bounces, so ONE press can look like five.

  That does not matter for "is the button held down right now". It matters a
  lot when you are counting presses or stepping through a menu.

  So isPressed() takes an optional second number:

      Robot.isPressed(LB)        // instant, exactly what the pin says
      Robot.isPressed(LB, 20)    // ignore anything that has not held
                                 // steady for 20 milliseconds

  This sketch counts with both, side by side. Press the left button a few
  times and watch the two counters drift apart. The debounced one is right.

  Note: the debounced version needs calling regularly, which is why it lives
  in loop(). It is watching how long the reading has stayed the same.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <RCBoard.h>

RCBoard Robot;

int rawCount = 0;
int cleanCount = 0;

bool rawWasDown = false;
bool cleanWasDown = false;

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Serial.println("Press the left button. Right button resets the counters.");
}

void loop() {
  // Count only the moment it goes down, not every pass while it is held.
  bool rawDown = Robot.isPressed(LB);
  if (rawDown && !rawWasDown) rawCount++;
  rawWasDown = rawDown;

  bool cleanDown = Robot.isPressed(LB, 20);
  if (cleanDown && !cleanWasDown) cleanCount++;
  cleanWasDown = cleanDown;

  // Light LED1 whenever the button is properly down.
  if (cleanDown) Robot.ledOn(LED1);
  else           Robot.ledOff(LED1);

  // Only print when something changed, or the Monitor floods.
  static int lastRaw = -1, lastClean = -1;
  if (rawCount != lastRaw || cleanCount != lastClean) {
    Serial.print("no debounce: ");
    Serial.print(rawCount);
    Serial.print("   with debounce: ");
    Serial.println(cleanCount);
    lastRaw = rawCount;
    lastClean = cleanCount;
  }

  if (Robot.isPressed(RB, 20)) {
    rawCount = 0;
    cleanCount = 0;
    Robot.ledOff(LED1);
  }
}
