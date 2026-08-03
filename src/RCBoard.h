/*
  RCBoard - Techgeeks Blueprint 01 Robot Controller Board

  One library for everything built into the board: two DC motors through the
  TB6612FNG driver, two push buttons, and two indicator LEDs.

  Every pin on this board is fixed in the PCB itself, so you never have to
  tell the library which pin anything is on. Just call begin().
*/

#ifndef RCBOARD_H
#define RCBOARD_H

#include <Arduino.h>

// Motor driver pins - wired into the PCB, do not change these.
#define PWMA  3
#define AIN2  4
#define AIN1  5
#define STDBY 6
#define BIN1  7
#define BIN2  8
#define PWMB  9

// Buttons: LB is the left button, RB is the right button.
#define LB  2
#define RB  10

// The two indicator LEDs.
#define LED1 11
#define LED2 12

class RCBoard {
  public:
    // Gets the library ready with sensible starting settings.
    RCBoard();

    // Sets up every pin and switches the motor driver on. Call this once.
    void begin();

    // Drives both motors. -255 to 255 each. 0 brakes that motor.
    void drive(int speedA, int speedB);

    // Brakes both motors and stops the robot.
    void stop();

    // True while the button is held down. Pass LB or RB.
    bool isPressed(int button, unsigned int debounceMs = 0);

    // Pauses your sketch until the button is pressed. Pass LB or RB.
    void waitFor(int button);

    // Turns an LED on. Pass LED1 or LED2.
    void ledOn(int led);

    // Turns an LED off. Pass LED1 or LED2.
    void ledOff(int led);

    // Flips an LED: on becomes off, off becomes on. Pass LED1 or LED2.
    void ledToggle(int led);

  private:
    bool _began;             // has begin() been called?

    bool _ledState[2];       // what we last told each LED to do

    // Only used when you ask isPressed() for a debounce.
    bool          _lastRaw[2];     // the reading we saw last time
    bool          _stable[2];      // the reading that has settled down
    unsigned long _lastChange[2];  // when the reading last flipped

    // Warnings print only once, so one mistake cannot flood the
    // Serial Monitor thousands of times a second.
    bool _warnedNoBegin;
    bool _warnedButton;
    bool _warnedLed;

    // Sends one motor forwards, backwards, or into a brake.
    void _driveMotor(int in1, int in2, int pwm, int speed);

    // Turns LB into 0 and RB into 1. Gives -1 and warns for anything else.
    int _buttonIndex(int button);

    // Turns LED1 into 0 and LED2 into 1. Gives -1 and warns for anything else.
    int _ledIndex(int led);

    // Reads a button right now. True means pressed.
    bool _readButton(int button);

    // True if begin() has run. Warns once if it has not.
    bool _checkBegan();
};

#endif
