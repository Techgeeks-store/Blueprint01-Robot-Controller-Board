# RCBoard

The official Arduino library for the **Blueprint 01 Robot Controller Board** by TECHGEEKS.

One library, one class, for everything built into the board: two DC motors through the TB6612FNG driver, two push buttons, and two indicator LEDs.

Every pin is fixed in the PCB itself, so **`begin()` takes no arguments**. You never look up a pin number.

---

## Contents

- [What the board is](#what-the-board-is)
- [Installing the library](#installing-the-library)
- [Your first sketch](#your-first-sketch)
- [Pin names](#pin-names)
- [Motors](#motors)
- [Buttons](#buttons)
- [LEDs](#leds)
- [Full function reference](#full-function-reference)
- [Example sketches](#example-sketches)
- [Warning messages and what to do about them](#warning-messages-and-what-to-do-about-them)
- [Using it with a sensor array](#using-it-with-a-sensor-array)
- [Things to know](#things-to-know)
- [License](#license)

---

## What the board is

The Blueprint 01 Robot Controller Board is a 5 × 6 cm PCB that replaces a breadboard full of loose wiring. It holds an **Arduino Nano** as the brain and a **TB6612FNG** motor driver for two DC motors, plus buttons, LEDs, a sensor rail, and screw terminals for motors and battery.

This library covers the parts that are wired into the board. Your sensor array is separate — see [Using it with a sensor array](#using-it-with-a-sensor-array).

| | |
|---|---|
| Input voltage | 6V to 14V |
| Motor channels | 2 (differential drive) |
| Continuous current per channel | 1.2 A |
| Sensor rail | 8 ADC pins (A0–A7) |
| Buttons | 2 (LB, RB) |
| LEDs | 2 (LED1, LED2) |

---

## Installing the library

### Option 1 — Add .ZIP Library (recommended)

1. Download **`RCBoard.zip`** from the [Releases page](https://github.com/Techgeeks-store/Blueprint01-Robot-Controller-Board/releases).
2. In the Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library…**
3. Select the file you downloaded.

Restart the IDE, and the examples appear under **File → Examples → RCBoard**.

### Option 2 — Download the whole repository

1. Click the green **Code** button at the top of this page, then **Download ZIP**.
2. Unzip it. You will get a folder called **`Blueprint01-Robot-Controller-Board-main`**.
3. **Rename that folder to just `RCBoard`.** This matters — the Arduino IDE wants the folder name to match the library name, and it does not like the dashes.
4. Move the `RCBoard` folder into your sketchbook's `libraries` folder:
   - **Windows:** `Documents\Arduino\libraries\`
   - **macOS:** `~/Documents/Arduino/libraries/`
   - **Linux:** `~/Arduino/libraries/`
5. Restart the Arduino IDE.

### Option 3 — Git clone

```bash
git clone https://github.com/Techgeeks-store/Blueprint01-Robot-Controller-Board.git RCBoard
```

Run that inside your `Arduino/libraries` folder. Note the `RCBoard` on the end — it clones into a correctly named folder for you.

### Checking it worked

Open **File → Examples → RCBoard → BasicMotorTest**. If it appears and compiles, you are ready.

---

## Your first sketch

```cpp
#include <RCBoard.h>

RCBoard Robot;

void setup() {
  Serial.begin(9600);   // you call this yourself
  Robot.begin();        // sets up every pin, switches the driver on
}

void loop() {
  Robot.drive(150, 150);   // both motors forward
  delay(1000);
  Robot.stop();            // brake
  delay(1000);
}
```

> **`begin()` fixes the most common problem with this board.** The TB6612FNG has a master on/off pin called STDBY. While it is LOW, both motors are dead no matter what else you send them — and forgetting to set it is the number one reason a freshly built robot just sits there. `begin()` does it for you, so it cannot be forgotten.

The library never calls `Serial.begin()` — that stays yours, so you pick the speed.

---

## Pin names

The library defines these for you, matching the labels on the board. Use the **names**, not the numbers.

| Name | Pin | What it is |
|---|---|---|
| `LB` | D2 | Left button |
| `RB` | D10 | Right button |
| `LED1` | D11 | First indicator LED |
| `LED2` | D12 | Second indicator LED |
| `PWMA` | D3 | Motor A speed |
| `AIN2` | D4 | Motor A direction |
| `AIN1` | D5 | Motor A direction |
| `STDBY` | D6 | Motor driver on/off |
| `BIN1` | D7 | Motor B direction |
| `BIN2` | D8 | Motor B direction |
| `PWMB` | D9 | Motor B speed |

You will only ever type `LB`, `RB`, `LED1` and `LED2` yourself. The motor pins are listed for reference — the library drives those.

---

## Motors

```cpp
Robot.drive(speedA, speedB);
```

Each speed runs from **−255 to 255**. Motor A is the **left** motor, Motor B is the **right** one.

| Call | What it does |
|---|---|
| `Robot.drive(150, 150)` | forward |
| `Robot.drive(-150, -150)` | backward |
| `Robot.drive(150, 50)` | turn right (left wheel faster) |
| `Robot.drive(50, 150)` | turn left (right wheel faster) |
| `Robot.drive(-150, 150)` | spin on the spot |
| `Robot.drive(0, 0)` | brake both |
| `Robot.stop()` | the same as `drive(0, 0)` |

**A speed of 0 brakes.** The motor actively resists turning and the robot stops where you told it to, rather than rolling on. That is almost always what you want on a robot.

**Numbers past the limit are quietly trimmed.** `drive(1000, 1000)` runs at 255, and `drive(-1000, 0)` at full reverse. No error, no surprise — handy when your PID maths occasionally overshoots.

### If the robot drives the wrong way

Run the **BasicMotorTest** example and watch what it actually does:

| What you see | Fix |
|---|---|
| Drives **backward** when it says forward | Swap the wires on **both** motor terminals |
| **Turns** when it says forward | One motor is mounted the other way round — swap the wires on **that motor only** |

Both fixes are at the screw terminals. No code change needed.

---

## Buttons

```cpp
if (Robot.isPressed(LB)) {
  // the left button is being held down right now
}
```

`isPressed()` returns **`true` when the button is pressed** — which sounds obvious, but the raw pin does the opposite.

<details>
<summary>Why the raw pin is backwards</summary>

The buttons use `INPUT_PULLUP`. The Arduino holds the pin HIGH by itself, and pressing the button connects it to GND, pulling it LOW. So `digitalRead(LB)` gives you **1 when released and 0 when pressed**. `isPressed()` flips that round so your `if` statements read the way you would say them out loud.
</details>

### Waiting for a press

```cpp
Robot.waitFor(LB);   // sketch pauses right here until pressed
```

This is the classic competition-robot startup: put the robot down, press a button when you are ready. See the **StartupSequence** example.

### Debouncing

```cpp
Robot.isPressed(LB)       // instant - exactly what the pin says
Robot.isPressed(LB, 20)   // ignore anything not steady for 20 ms
```

A push button is two pieces of metal springing together. For a few milliseconds after you press it, they bounce apart and back several times — and the Arduino is fast enough to see every bounce. **One press can look like five.**

| Use | Which |
|---|---|
| "is it held down right now" | `isPressed(LB)` — no debounce needed |
| Counting presses | `isPressed(LB, 20)` |
| Stepping through a menu | `isPressed(LB, 20)` |
| Toggling something on and off | `isPressed(LB, 20)` |

**20 ms suits most buttons.** Raise it if presses still double up, lower it if the button feels sluggish.

Two things to know about the debounced version:

- **Call it regularly**, ideally every pass through `loop()`. It works by watching how long the reading has stayed the same, so it needs to keep looking. Calling it once every few seconds will not work properly.
- **Each button remembers separately**, so debouncing `LB` never disturbs `RB`.

The **Debounce** example counts presses both ways side by side, so you can watch the two counters drift apart.

---

## LEDs

```cpp
Robot.ledOn(LED1);
Robot.ledOff(LED1);
Robot.ledToggle(LED1);   // on becomes off, off becomes on
```

Both LEDs start **off** after `begin()`.

They are the fastest way to see what your robot is thinking — light one when calibration starts, another when the line is lost. Far easier than reading the Serial Monitor while the robot is driving around.

---

## Full function reference

| Function | What it does |
|---|---|
| `RCBoard()` | Creates the object. Put `RCBoard Robot;` above `setup()`. |
| `void begin()` | Sets up every pin and switches the motor driver on. Call once in `setup()`. |
| `void drive(int speedA, int speedB)` | Drives both motors, −255 to 255 each. 0 brakes. Out-of-range values are trimmed silently. |
| `void stop()` | Brakes both motors. Same as `drive(0, 0)`. |
| `bool isPressed(int button, unsigned int debounceMs = 0)` | `true` while held. Pass `LB` or `RB`. Add a debounce time in milliseconds when counting presses. |
| `void waitFor(int button)` | Pauses the sketch until that button is pressed. Pass `LB` or `RB`. |
| `void ledOn(int led)` | Turns an LED on. Pass `LED1` or `LED2`. |
| `void ledOff(int led)` | Turns an LED off. Pass `LED1` or `LED2`. |
| `void ledToggle(int led)` | Flips an LED. Pass `LED1` or `LED2`. |

---

## Example sketches

All under **File → Examples → RCBoard**.

| Example | What it teaches |
|---|---|
| **BasicMotorTest** | Every direction in turn, so you can check your motor wiring. **Run this first.** |
| **ButtonsAndLEDs** | Reading both buttons and driving both LEDs. |
| **Debounce** | Counting presses with and without debouncing, side by side. |
| **StartupSequence** | The competition pattern: wait for a button, set up, wait again, then drive. |

---

## Warning messages and what to do about them

The library speaks up in the Serial Monitor when something is wrong. **Each message prints only once per run**, so a mistake inside `loop()` cannot flood the Monitor and hide everything else.

| Message | What went wrong | How to fix it |
|---|---|---|
| `RCBoard: call begin() first` | You used the board before setting it up. | Add `Robot.begin();` to `setup()`. |
| `RCBoard: unknown button` | Something other than `LB` or `RB` was passed. | Use the names `LB` and `RB`. |
| `RCBoard: unknown LED` | Something other than `LED1` or `LED2` was passed. | Use the names `LED1` and `LED2`. |

### Something is wrong but there is no message

| What you see | Almost certainly |
|---|---|
| **Nothing at all in the Serial Monitor** | No `Serial.begin(9600);` in `setup()`, or the Monitor's speed box does not match. |
| **Motors do not spin, no message** | Battery flat, power switch off, or loose screw terminals. `begin()` already handles STDBY, so that is not it. |
| **One motor spins, the other does not** | Loose screw terminal on the dead one, or that motor's wiring. |
| **Robot drives the wrong way** | See the motor wiring table above — it is a terminal swap, not a code change. |
| **Board does not power on** | Reversed battery polarity, or a flat battery. Turn it off and check + and −. |
| **A button seems to fire several times** | Contact bounce. Use `isPressed(LB, 20)` instead of `isPressed(LB)`. |
| **Sensor values jump about while driving** | Motor noise. Check the sensor rail connections and that the board's capacitor is seated. |

---

## Using it with a sensor array

RCBoard drives the robot; a sensor library reads the track. They are separate libraries and work together happily:

```cpp
#include <RCBoard.h>
#include <ARC8.h>

RCBoard Robot;
ARC8 sensors;

int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int values[8];

void setup() {
  Serial.begin(9600);
  Robot.begin();
  sensors.begin(pins);
  sensors.setPID(0.03, 0.0, 0.02);

  Robot.waitFor(LB);          // press left to calibrate
  Robot.ledOn(LED1);
  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }
  Robot.ledOff(LED1);

  Robot.waitFor(RB);          // press right to go
  Robot.ledOn(LED2);
}

void loop() {
  sensors.read(values);
  int correction = sensors.getCorrection();
  Robot.drive(120 + correction, 120 - correction);
}
```

That is the whole line follower. The sensor rail exposes **A0–A7**, so it fits an [ARC8](https://github.com/Techgeeks-store/ARC8-Sensor-Library) directly.

> **A6 and A7 are analog-only on the Arduino Nano.** They cannot be used as digital pins. That does not affect the ARC8, which only ever reads them as analog — but it does mean any sensor needing digital pins on the rail is limited to **A0–A5**. The [ARC16](https://github.com/Techgeeks-store/ARC16-Sensor-Library) needs digital select lines, so wire those to A0–A5 or to spare digital pins.

---

## Things to know

- **All the pins are fixed.** They are etched into the PCB, so there is nothing to configure and nothing to get wrong.
- **`begin()` handles STDBY.** The single most common motor problem on this board cannot happen when you use this library.
- **0 brakes, it does not coast.** There is no coast option — braking gives better control, which is what the board's own manual recommends.
- **`waitFor()` blocks on purpose.** Nothing else in your sketch runs while it waits. That is the point, but do not put it inside `loop()` unless you mean it.
- **`LB`, `RB`, `LED1`, `LED2` and the motor pin names are global.** If another library defines any of the same names, you will get a "redefined" warning from the compiler.
- **No memory tricks.** No `malloc`, no `new`, no `String`. Everything is fixed size, so it cannot fragment your RAM mid-race.
- **No `delay()` inside the library.** The debounce uses `millis()`, and `waitFor()` spins on the pin. Neither one blocks with `delay()`.

### Memory use

Measured on an Arduino Nano, compiled with the real Arduino AVR core:

| Example | Flash | RAM |
|---|---|---|
| ButtonsAndLEDs | 3,298 bytes (10.1%) | 308 bytes (15.0%) |
| BasicMotorTest | 3,508 bytes (10.7%) | 344 bytes (16.8%) |
| Debounce | 3,874 bytes (11.8%) | 306 bytes (14.9%) |
| StartupSequence | 4,210 bytes (12.8%) | 344 bytes (16.8%) |

The library itself is about 1.1 KB of flash, leaving plenty for your sensor library and your own code.

---

## License

MIT — see [LICENSE](LICENSE). Use it, change it, build products with it. Just keep the copyright notice.

Copyright © 2026 TECHGEEKS
